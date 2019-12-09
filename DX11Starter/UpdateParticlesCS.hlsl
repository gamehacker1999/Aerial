struct Particle
{
	float spawnTime;
	float3 startPosition;

	float rotationStart;
	float3 startVelocity;

	float rotationEnd;
	float age;

	float2 padding;
};

struct ParticleDrawData
{
	uint  index;
	float distanceFromCam; //distance from camera for sorting the particles
};

cbuffer exterData: register(b0)
{
	int maxParticles;
	float currentTime;
	float deltaTime;
	float lifeTime;
	float3 acceleration;
}

RWStructuredBuffer<Particle> particlePool : register(u0);
AppendStructuredBuffer<uint> deadList: register(u1);
RWStructuredBuffer<ParticleDrawData> drawIndices: register(u2);

[numthreads(32, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
	if (id.x >= maxParticles) return;

	uint index = id.x;
	Particle p = particlePool.Load(index);
	float age = currentTime - p.spawnTime;

	if (p.age == 0.0f)
	{
		return;
	}

	if (age >lifeTime)
	{
		p.age = 0;
	}

	particlePool[index] = p;

	if (p.age = 0)
	{
		deadList.Append(id.x);
	}

	else
	{
		uint drawIndex = drawIndices.IncrementCounter();

		ParticleDrawData particleDraw;
		particleDraw.index = index;
		particleDraw.distanceFromCam = 0;

		drawIndices[drawIndex] = particleDraw;
	}
}