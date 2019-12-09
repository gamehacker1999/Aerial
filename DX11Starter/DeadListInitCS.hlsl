cbuffer externDate: register(b0)
{
	int maxParticles;
}

struct Particle
{
	float spawnTime;
	float3 startPosition;

	float rotationStart;
	float3 startVelocity;

	float rotationEnd;
	float3 padding;
};

AppendStructuredBuffer<uint> particleDeadList: register(u0);

[numthreads(32, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
	if (id.x >= maxParticles)
		return;

	//adding dead list variable
	particleDeadList.Append(id.x);
}