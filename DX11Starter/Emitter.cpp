#include "Emitter.h"

Emitter::Emitter(int maxParticles,
	int particlesPerSecond,
	float lifetime,
	float startSize,
	float endSize,
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
	SimpleComputeShader* deadListInitCS,
	SimpleComputeShader* updateCS,
	SimpleComputeShader* emitCS,
	SimpleComputeShader* drawCopyCS,
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
	this->deadListInitCS = deadListInitCS;
	this->emitCS = emitCS;
	this->copyDrawCS = drawCopyCS;
	this->updateCS = updateCS;
	this->ps = ps;
	this->vs = vs;
	this->texture = texture;

	timeSinceEmit = 0;//how long since the last particle was emmited
	livingParticleCount = 0; //count of how many particles
	//circular buffer indices
	firstAliveIndex = 0;
	firstDeadIndex = 0;
	isDead = false;
	isTemp = false;
	emitterAge = true;
	explosive = false;

	particles = new Particle[maxParticles];//list of particles
	ZeroMemory(particles, sizeof(Particle) * maxParticles);

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

	//create a structured buffer to hold the particles
	D3D11_BUFFER_DESC particleBufferDesc = {};
	particleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS; //bind it to srv
	particleBufferDesc.ByteWidth = maxParticles*sizeof(Particle);
	particleBufferDesc.CPUAccessFlags = 0;
	particleBufferDesc.StructureByteStride = sizeof(Particle); //sizeof structure
	particleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	particleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; //this is a structured buffer
	//creat the buffer
	device->CreateBuffer(&particleBufferDesc, 0, &particleBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC particleSRV = {}; //srv for particles
	particleSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	particleSRV.Format = DXGI_FORMAT_UNKNOWN;
	particleSRV.Buffer.FirstElement = 0;
	particleSRV.Buffer.NumElements = maxParticles;	

	device->CreateShaderResourceView(particleBuffer, &particleSRV, &particleData);

	//uav to represent the particles
	D3D11_UNORDERED_ACCESS_VIEW_DESC poolUAVDesc = {};
	poolUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // Needed for RW structured buffers
	poolUAVDesc.Buffer.FirstElement = 0;
	poolUAVDesc.Buffer.Flags = 0;
	poolUAVDesc.Buffer.NumElements = maxParticles;
	poolUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(particleBuffer, &poolUAVDesc, particlePoolUAV.GetAddressOf());

	//dead list
	ID3D11Buffer* deadListBuffer;
	D3D11_BUFFER_DESC deadDesc = {};
	deadDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	deadDesc.ByteWidth = sizeof(unsigned int) * maxParticles;
	deadDesc.CPUAccessFlags = 0;
	deadDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	deadDesc.StructureByteStride = sizeof(unsigned int);
	deadDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateBuffer(&deadDesc, 0, &deadListBuffer);

	// UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC deadUAVDesc = {};
	deadUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // Needed for RW structured buffers
	deadUAVDesc.Buffer.FirstElement = 0;
	deadUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // Append/Consume
	deadUAVDesc.Buffer.NumElements = maxParticles;
	deadUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(deadListBuffer, &deadUAVDesc, particleDeadUAV.GetAddressOf());

	deadListBuffer->Release();

	//draw list 
			// Buffer
	ID3D11Buffer* drawListBuffer;
	D3D11_BUFFER_DESC drawDesc = {};
	drawDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	drawDesc.ByteWidth = sizeof(ParticleSort) * maxParticles;
	drawDesc.CPUAccessFlags = 0;
	drawDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	drawDesc.StructureByteStride = sizeof(ParticleSort);
	drawDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateBuffer(&drawDesc, 0, &drawListBuffer);

	// UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC drawUAVDesc = {};
	drawUAVDesc.Format = DXGI_FORMAT_UNKNOWN; // Needed for RW structured buffers
	drawUAVDesc.Buffer.FirstElement = 0;
	drawUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER; // IncrementCounter() in HLSL
	drawUAVDesc.Buffer.NumElements = maxParticles;
	drawUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(drawListBuffer, &drawUAVDesc, &particleDrawUAV);

	// SRV (for indexing in VS)
	D3D11_SHADER_RESOURCE_VIEW_DESC drawSRVDesc = {};
	drawSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	drawSRVDesc.Buffer.FirstElement = 0;
	drawSRVDesc.Buffer.NumElements = maxParticles;
	// Don't actually set these!  They're union'd with above data, so 
	// it will just overwrite correct values with incorrect values
	//drawSRVDesc.Buffer.ElementOffset = 0;
	//drawSRVDesc.Buffer.ElementWidth = sizeof(ParticleSort);
	drawSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	device->CreateShaderResourceView(drawListBuffer, &drawSRVDesc, &particleDrawSRV);

	// Done
	drawListBuffer->Release();

	//draw args
			// Buffer
	D3D11_BUFFER_DESC argsDesc = {};
	argsDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	argsDesc.ByteWidth = sizeof(unsigned int) * 5; // Need 5 if using an index buffer!
	argsDesc.CPUAccessFlags = 0;
	argsDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	argsDesc.StructureByteStride = 0;
	argsDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateBuffer(&argsDesc, 0, drawArgsBuffer.GetAddressOf());

	// UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC argsUAVDesc = {};
	argsUAVDesc.Format = DXGI_FORMAT_R32_UINT; // Actually UINT's in here!
	argsUAVDesc.Buffer.FirstElement = 0;
	argsUAVDesc.Buffer.Flags = 0;  // Nothing special
	argsUAVDesc.Buffer.NumElements = 5; // Need 5 if using an index buffer
	argsUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(drawArgsBuffer.Get(), &argsUAVDesc, &drawArgsUAV);

	// Must keep buffer ref for indirect draw!

	//dead list initialize
	deadListInitCS->SetShader();
	deadListInitCS->SetInt("maxParticles", maxParticles);
	deadListInitCS->SetUnorderedAccessView("particleDeadList", particleDeadUAV.Get());
	deadListInitCS->CopyAllBufferData();
	deadListInitCS->DispatchByThreads(maxParticles, 1, 1);

	delete[] indices;
}

Emitter::~Emitter()
{
	indexBuffer->Release();
	particleBuffer->Release();
	particleData->Release();
	delete[] particles;
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

void Emitter::UpdateParticles(float deltaTime, float currentTime, ID3D11DeviceContext* context)
{

	if (isTemp)
	{
		emitterAge += deltaTime;
		if (emitterAge >= emitterLifetime)
		{
			isDead = true;
		}
	}
	// Reset UAVs (potential issue with setting the following ones)
	ID3D11UnorderedAccessView* none[8] = {};
	context->CSSetUnorderedAccessViews(0, 8, none, 0);
	timeSinceEmit += deltaTime;

	while (timeSinceEmit >= secondsPerParticle)
	{
		// How many to emit?
		int emitCount = (int)(timeSinceEmit / secondsPerParticle);

		// Max to emit in a single batch is 65,535
		emitCount = min(emitCount, 65535);

		// Adjust time counter
		timeSinceEmit = fmod(timeSinceEmit, secondsPerParticle);

		// Emit an appropriate amount of particles
		emitCS->SetShader();
		emitCS->SetFloat("totalTime", currentTime);
		emitCS->SetInt("emitCount", emitCount);
		emitCS->SetFloat3("emitterPos", emitterPosition);
		emitCS->SetFloat3("startVelocity", startVelocity);
		emitCS->SetFloat3("posRandomRange", positionRandomRange);
		emitCS->SetFloat3("velRandomRange", velocityRandomRange);
		emitCS->SetFloat4("rotRandomRange", rotationRandomRanges);
		emitCS->SetUnorderedAccessView("ParticlePool", particlePoolUAV.Get());
		emitCS->SetUnorderedAccessView("DeadList", particleDeadUAV.Get());
		emitCS->CopyAllBufferData();
		emitCS->DispatchByThreads(emitCount, 1, 1);
	}

	context->CSSetUnorderedAccessViews(0, 8, none, 0);

	// Update
	updateCS->SetShader();
	updateCS->SetFloat("deltaTime", deltaTime);
	updateCS->SetFloat("currentTime", currentTime);
	updateCS->SetFloat("lifeTime", lifetime);
	updateCS->SetFloat3("acceleration", emitterAcceleration);
	updateCS->SetInt("maxParticles", maxParticles);
	updateCS->SetUnorderedAccessView("particlePool", particlePoolUAV.Get());
	updateCS->SetUnorderedAccessView("deadList", particleDeadUAV.Get());
	updateCS->SetUnorderedAccessView("drawIndices", particleDrawUAV.Get(), 0); // Reset counter for update!

	updateCS->CopyAllBufferData();
	updateCS->DispatchByThreads(maxParticles, 1, 1);

	context->CSSetUnorderedAccessViews(0, 8, none, 0);

	// Get draw data
	copyDrawCS->SetShader();
	copyDrawCS->CopyAllBufferData();
	copyDrawCS->SetUnorderedAccessView("drawArgs", drawArgsUAV.Get());
	copyDrawCS->SetUnorderedAccessView("drawList", particleDrawUAV.Get()); // Don't reset counter!!!
	copyDrawCS->DispatchByThreads(1, 1, 1);

	// Reset here too
	context->CSSetUnorderedAccessViews(0, 8, none, 0);

	// Binding order issues with next stage, so just reset here

	/*if (livingParticleCount > 0)
	{
		//looping through the circular buffer
		if (firstAliveIndex < firstDeadIndex)
		{
			for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}
		}

		//if firse alive is ahead
		else
		{
			//go from the first alive to end of list
			for (int i = firstAliveIndex; i < maxParticles; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}

			//go from zero to dead
			for (int i = 0; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}
		}
	}*/




}

void Emitter::Draw(ID3D11DeviceContext* context, XMFLOAT4X4 view, XMFLOAT4X4 projection, float currentTime)
{
	//mapping the data so that gpu cannot write to it
	/*D3D11_MAPPED_SUBRESOURCE mapped = {};

	//mapping the vertex buffer so that cpu can write to it
	context->Map(particleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	//copying the data from cpu to gpu
	memcpy(mapped.pData, particles, sizeof(Particle) * maxParticles);

	//unmapping the resource
	context->Unmap(particleBuffer, 0);*/

	//setting the up the buffer
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer=nullptr;
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);

	//setting the view and projection matrix
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", projection);
	vs->SetFloat3("acceleration", emitterAcceleration);
	vs->SetFloat4("startColor", startColor);
	vs->SetFloat4("endColor", endColor);
	vs->SetFloat("startSize", startSize);
	vs->SetFloat("endSize", endSize);
	vs->SetFloat("lifetime",lifetime);
	vs->SetFloat("currentTime",currentTime);
	vs->SetShader();
	
	context->VSSetShaderResources(0, 1, &particleData);
	context->VSSetShaderResources(1, 1, particleDrawSRV.GetAddressOf());
	vs->CopyAllBufferData();


	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();
	context->DrawIndexedInstancedIndirect(drawArgsBuffer.Get(), 0);

	ID3D11ShaderResourceView* none[16] = {};
	context->VSSetShaderResources(0, 16, none);

	/*if (firstAliveIndex < firstDeadIndex)
	{
		vs->SetInt("startIndex", firstAliveIndex);
		vs->CopyAllBufferData();
		context->DrawIndexed(livingParticleCount * 6, 0, 0);
	}

	else
	{
		//draw from 0 to dead
		vs->SetInt("startIndex", 0);
		vs->CopyAllBufferData();
		context->DrawIndexed(firstDeadIndex*6, 0, 0);

		//draw from alive to max
		vs->SetInt("startIndex", firstAliveIndex);
		vs->CopyAllBufferData();
		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, 0, 0);
	}*/

}

void Emitter::SetTemporary(float emitterLife)
{
	isTemp = true;
	this->emitterLifetime = emitterLife;
}

bool Emitter::IsDead()
{
	return isDead;
}

void Emitter::Explosive()
{
	explosive = true;
}

void Emitter::UpdateSingleParticle(float dt, int index,float currentTime)
{
	float age = currentTime - particles[index].spawnTime;

	//if age exceeds its lifespan
	if (age >= lifetime)
	{
		//increase the first alive index
		firstAliveIndex++;
		//wrap it
		firstAliveIndex %= maxParticles;
		//kill this particle
		livingParticleCount--;
		return;
	}
}

void Emitter::SpawnParticle(float currentTime)
{
	if (livingParticleCount == maxParticles)
		return;

	//spawinig a new particle
	particles[firstDeadIndex].spawnTime = currentTime;

	//random position and veloctiy of the particle
	std::random_device rd;
	std::mt19937 randomGenerator(rd());
	std::uniform_real_distribution<float> dist(-1, 1);

	particles[firstDeadIndex].startPosition = emitterPosition; //particles start at emitter
	//randomizing their x,y,z
	particles[firstDeadIndex].startPosition.x += dist(randomGenerator) * positionRandomRange.x;
	particles[firstDeadIndex].startPosition.y += dist(randomGenerator) * positionRandomRange.y;
	particles[firstDeadIndex].startPosition.z += dist(randomGenerator) * positionRandomRange.z;

	particles[firstDeadIndex].startVelocity = startVelocity;
	particles[firstDeadIndex].startVelocity.x += dist(randomGenerator) * velocityRandomRange.x;
	particles[firstDeadIndex].startVelocity.y += dist(randomGenerator) * velocityRandomRange.y;
	particles[firstDeadIndex].startVelocity.z += dist(randomGenerator) * velocityRandomRange.z;

	//random start rotation
	float rotStartMin = rotationRandomRanges.x;
	float rotStartMax = rotationRandomRanges.y;

	//choosing a random start rotation
	particles[firstDeadIndex].rotationStart = dist(randomGenerator) * (rotStartMax - rotStartMin) + rotStartMin;

	//random start rotation
	float rotEndMin = rotationRandomRanges.z;
	float rotEndMax = rotationRandomRanges.w;

	//choosing a random start rotation
	particles[firstDeadIndex].rotationEnd = dist(randomGenerator) * (rotEndMax - rotEndMin) + rotEndMin;

	//increment the first dead index
	firstDeadIndex++;
	firstDeadIndex %= maxParticles;

	//increment living particles
	livingParticleCount++;

}

