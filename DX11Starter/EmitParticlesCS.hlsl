struct Particle
{
	float spawnTime;
	float3 startPosition;

	float rotationStart;
	float3 startVelocity;

	float rotationEnd;
	float3 padding;
};

cbuffer externalData : register(b0)
{
	float totalTime;
	int emitCount;
	float3 emitterPos;
	float3 startVelocity;
	float3 posRandomRange;
	float3 velRandomRange;
	float4 rotRandomRange;
	float4 rotations;
}

// Order should match UpdateCS (RW binding issues)
RWStructuredBuffer<Particle> ParticlePool	: register(u0);
ConsumeStructuredBuffer<uint> DeadList		: register(u1);

//wand hash algorithm to use for an xor shift random number seed
uint WangHash(uint seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

uint RandXorShift(uint rng_state)
{
	// Xorshift algorithm from George Marsaglia's paper
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return rng_state;
}


[numthreads(32, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{

	if (id.x >= emitCount) return;

	//creating 8 random numbers
	float randNum = (float)id.x;
	randNum = WangHash(randNum);
	randNum = RandXorShift(randNum);
	randNum *=(1.0 / 4294967296.0);
	randNum = randNum * 2.0 - 1.0;

	uint index = DeadList.Consume();

	Particle p;
	p.spawnTime = totalTime;
	p.startPosition = emitterPos;
	p.startPosition.x += randNum * posRandomRange.x;
	p.startPosition.y += randNum * posRandomRange.y;
	p.startPosition.z += randNum * posRandomRange.z;

	p.startVelocity = startVelocity;
	p.startVelocity.x += velRandomRange.x * randNum;
	p.startVelocity.y += velRandomRange.y * randNum;
	p.startVelocity.z += velRandomRange.z * randNum;

	//random start rotation
	float rotStartMin = rotRandomRange.x;
	float rotStartMax = rotRandomRange.y;

	//choosing a random start rotation
	p.rotationStart = randNum * (rotStartMin - rotStartMax) + rotStartMin;

	//random start rotation
	float rotEndMin = rotRandomRange.z;
	float rotEndMax = rotRandomRange.w;

	//choosing a random start rotation
	p.rotationEnd = randNum * (rotEndMin - rotEndMax) + rotEndMin;
	p.padding = float3(0, 0, 0);

	ParticlePool[index] = p;

}