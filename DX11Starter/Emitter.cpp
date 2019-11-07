#include "Emitter.h"

Emitter::Emitter(int maxParticles, int particlesPerSecond, 
	float lifetime, float startSize, float endSize, 
	XMFLOAT4 startColor, 
	XMFLOAT4 endColor, 
	XMFLOAT3 startVelocity, 
	XMFLOAT3 velocityRandomRange, 
	XMFLOAT3 emitterPosition, 
	XMFLOAT3 positionRandomRange, 
	XMFLOAT4 rotationRandomRanges, 
	XMFLOAT3 emitterAcceleration, 
	ID3D11Device* device, 
	SimpleVertexShader* vs, 
	SimplePixelShader* ps, 
	ID3D11ShaderResourceView* texture)
{
	this->maxParticles = maxParticles; //max particles spewed
	this->particlesPerSecond = particlesPerSecond; //particles spewed per second
	this->secondsPerParticle = 1.0f / particlesPerSecond; //amount after which a particle is spawned
	this->lifetime = lifetime; //lifetime of each particle
	this->startSize = startSize; //start size
	this->endSize = endSize; //end size
	this->startColor = startColor; //start color to interpolate from
	this->endColor = endColor; //end color to interpolate to
	this->startVelocity = startVelocity; //start velocity
	this->velocityRandomRange = velocityRandomRange; //range of velocity
	this->emitterPosition = emitterPosition; //position of emitter
	this->positionRandomRange = positionRandomRange; //range of pos
	this->rotationRandomRanges = rotationRandomRanges; //random ranges of rotation
	this->emitterAcceleration = emitterAcceleration; //acceleration of emmiter
	this->ps = ps;
	this->vs = vs;
	this->texture = texture;

	timeSinceEmit = 0;//how long since the last particle was emmited
	livingParticleCount = 0; //count of how many particles
	//circular buffer indices
	firstAliveIndex = 0;
	firstDeadIndex = 0;

	particles = new Particle[maxParticles];//list of particles
	ZeroMemory(particles, sizeof(Particle) * maxParticles);
	particleVertices = new ParticleVertex[4*maxParticles]; //each particle has 4 vertices

	//create uvs for particles
	for (size_t i = 0; i < maxParticles*4; i+=4)
	{
		particleVertices[i + 0].uv = XMFLOAT2(0, 0);
		particleVertices[i + 1].uv = XMFLOAT2(1, 0);
		particleVertices[i + 2].uv = XMFLOAT2(1, 1);
		particleVertices[i + 3].uv = XMFLOAT2(0, 1);
	}

	//create a dynamic vertex buffer
	D3D11_BUFFER_DESC vbd = {};
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	device->CreateBuffer(&vbd, nullptr, &vertexBuffer);


	unsigned int* indices = new unsigned int[6 * maxParticles];
	int indexCount = 0;
	for (int i = 0; i < maxParticles*4; i+=4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i+1;
		indices[indexCount++] = i+2;
		indices[indexCount++] = i;
		indices[indexCount++] = i+2;
		indices[indexCount++] = i+3;

	}

	//creating subresource
	D3D11_SUBRESOURCE_DATA ibdSub = {};
	ibdSub.pSysMem =  (indices);

	//creating indexbuffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.CPUAccessFlags = 0;
	ibd.ByteWidth = 6 * sizeof(unsigned int) * maxParticles;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&ibd, &ibdSub, &indexBuffer);
	delete[] indices;
}

Emitter::~Emitter()
{
	vertexBuffer->Release();
	indexBuffer->Release();
	delete[] particles;
	delete[] particleVertices;
}

XMFLOAT3 Emitter::GetPosition()
{
	return emitterPosition;
}

void Emitter::SetPosition(XMFLOAT3 pos)
{
	emitterPosition = pos;
}

void Emitter::SetAcceleration(XMFLOAT3 acel)
{
	emitterAcceleration = acel;
}

void Emitter::UpdateParticles(float deltaTime)
{
	//looping through the circular buffer
	if (firstAliveIndex < firstDeadIndex) 
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
		{
			UpdateSingleParticle(deltaTime, i);
		}
	}

	//if firse alive is ahead
	else
	{
		//go from the first alive to end of list
		for (int i = firstAliveIndex; i < maxParticles; i++)
		{
			UpdateSingleParticle(deltaTime, i);
		}

		//go from zero to dead
		for (int i = 0; i < firstDeadIndex; i++)
		{
			UpdateSingleParticle(deltaTime, i);
		}
	}

	timeSinceEmit += deltaTime;

	while(timeSinceEmit >= secondsPerParticle)
	{
		SpawnParticle();
		timeSinceEmit -= secondsPerParticle;
	}

}

void Emitter::Draw(ID3D11DeviceContext* context, std::shared_ptr<Camera> camera)
{
	//copy to dynamic buffer
	CopyParticlesToGPU(context, camera);

	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;

	//setting the buffers
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//setting the view and projection matrix
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetShader();
	vs->CopyAllBufferData();

	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();

	if (firstAliveIndex < firstDeadIndex)
	{
		context->DrawIndexed(livingParticleCount * 6, firstAliveIndex*6, 0);
	}

	else
	{
		//draw from 0 to dead
		context->DrawIndexed(max(firstDeadIndex - 1, 0)*6, 0, 0);

		//draw from alive to max
		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, 6 * firstAliveIndex, 0);
	}

}

void Emitter::UpdateSingleParticle(float dt, int index)
{
	if (particles[index].age >= lifetime)
	{
		return;
	}

	//increment the particle age
	particles[index].age += dt;

	//if age exceeds its lifespan
	if (particles[index].age >= lifetime)
	{
		//increase the first alive index
		firstAliveIndex++;
		//wrap it
		firstAliveIndex %= maxParticles;
		//kill this particle
		livingParticleCount--;
		return;
	}

	float percent = particles[index].age / lifetime;

	//lerping for rotation, color, and size
	float rotStart = particles[index].rotStart;
	float rotEnd = particles[index].rotEnd;

	//current rotation
	particles[index].rot = rotStart + percent * (rotEnd - rotStart);

	//current size
	particles[index].size = startSize + percent*(endSize - startSize);

	//lerping between the start and end color
	XMStoreFloat4(&particles[index].color, XMVectorLerp(XMLoadFloat4(&startColor), XMLoadFloat4(&endColor), percent));

	//updating the position
	XMVECTOR startPos = XMLoadFloat3(&particles[index].startPos);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].startVelocity);
	XMVECTOR accel = XMLoadFloat3(&emitterAcceleration);

	//S=Si+Vi*t+0.5*acel*t*t, newton's third equation of motion
	XMVECTOR addPos = startPos + (startVel * particles[index].age) + (0.5 * accel * particles[index].age * particles[index].age);

	XMStoreFloat3(&particles[index].position, addPos);
}

void Emitter::SpawnParticle()
{
	if (livingParticleCount == maxParticles)
		return;

	//spawinig a new particle
	particles[firstDeadIndex].age = 0;
	particles[firstDeadIndex].size = startSize;
	particles[firstDeadIndex].color = startColor;

	//random position and veloctiy of the particle
	std::random_device rd;
	std::mt19937 randomGenerator(rd());
	std::uniform_real_distribution<float> dist(-1, 1);

	particles[firstDeadIndex].startPos = emitterPosition; //particles start at emitter
	//randomizing their x,y,z
	particles[firstDeadIndex].startPos.x += dist(randomGenerator) * positionRandomRange.x;
	particles[firstDeadIndex].startPos.y += dist(randomGenerator) * positionRandomRange.y;
	particles[firstDeadIndex].startPos.z += dist(randomGenerator) * positionRandomRange.z;

	particles[firstDeadIndex].position = particles[firstDeadIndex].startPos;

	particles[firstDeadIndex].startVelocity = startVelocity;
	particles[firstDeadIndex].startVelocity.x += dist(randomGenerator) * velocityRandomRange.x;
	particles[firstDeadIndex].startVelocity.y += dist(randomGenerator) * velocityRandomRange.y;
	particles[firstDeadIndex].startVelocity.z += dist(randomGenerator) * velocityRandomRange.z;

	//random start rotation
	float rotStartMin = rotationRandomRanges.x;
	float rotStartMax = rotationRandomRanges.y;

	//choosing a random start rotation
	particles[firstDeadIndex].rotStart = dist(randomGenerator) * (rotStartMax - rotStartMin) + rotStartMin;

	//random start rotation
	float rotEndMin = rotationRandomRanges.z;
	float rotEndMax = rotationRandomRanges.w;

	//choosing a random start rotation
	particles[firstDeadIndex].rotEnd = dist(randomGenerator) * (rotEndMax - rotEndMin) + rotEndMin;

	//increment the first dead index
	firstDeadIndex++;
	firstDeadIndex %= maxParticles;

	//increment living particles
	livingParticleCount++;

}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext* context, std::shared_ptr<Camera> camera)
{
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
		{
			CopyOneParticle(i, camera);
		}
	}

	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
		{
			CopyOneParticle(i, camera);
		}

		for (int i = 0; i < firstDeadIndex; i++)
		{
			CopyOneParticle(i, camera);
		}
	}

	//mapping the data so that gpu cannot write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	//mapping the vertex buffer so that cpu can write to it
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD,0,&mapped);

	//copying the data from cpu to gpu
	memcpy(mapped.pData, particleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	//unmapping the resource
	context->Unmap(vertexBuffer, 0);
}

void Emitter::CopyOneParticle(int index, std::shared_ptr<Camera> camera)
{
	int i = index * 4;

	//calculating the particle vertex position
	particleVertices[i].position = CalcParticleVertexPosition(index, 0, camera);
	particleVertices[i+1].position = CalcParticleVertexPosition(index, 1, camera);
	particleVertices[i+2].position = CalcParticleVertexPosition(index, 2, camera);
	particleVertices[i+3].position = CalcParticleVertexPosition(index, 3, camera);

	//calculating the colors for the particles
	particleVertices[i].color = particles[index].color;
	particleVertices[i+1].color = particles[index].color;
	particleVertices[i+2].color = particles[index].color;
	particleVertices[i+3].color = particles[index].color;

}

XMFLOAT3 Emitter::CalcParticleVertexPosition(int particleIndex, int quadCornerIndex, std::shared_ptr<Camera> camera)
{
	XMFLOAT4X4 view = camera->GetViewMatrix();
	//getting the right and up vector of camera for billboarding
	XMVECTOR up = XMVectorSet(view._11, view._12, view._13, 0);
	XMVECTOR right = XMVectorSet(view._21, view._22, view._23, 0);

	//getting the uv of this quad and calculating its offset
	XMFLOAT2 offset = particleVertices[particleIndex * 4 + quadCornerIndex].uv;
	offset.x = offset.x * 2 - 1; //unpacking it to -1 to 1
	offset.y = offset.y * -2 + 1; //unpacking and flipping the y

	//load into a vector and rotate it wrt z axis using the cutting rotation of the particle
	XMVECTOR offsetVec = XMLoadFloat2(&offset);
	XMMATRIX rotMat = XMMatrixRotationZ(particles[particleIndex].rot);
	offsetVec = XMVector3Transform(offsetVec, rotMat);

	//add and scale the camera up and right to positon
	XMVECTOR posVec=XMLoadFloat3(&particles[particleIndex].position);
	posVec += right * XMVectorGetX(offsetVec) * particles[particleIndex].size;
	posVec += up * XMVectorGetY(offsetVec) * particles[particleIndex].size;

	XMFLOAT3 pos;
	XMStoreFloat3(&pos, posVec);
	return pos;
}
