struct ParticleDrawData
{
	uint  index;
	float distanceFromCam; //distance from camera for sorting the particles
};

RWBuffer<uint>						drawArgs	: register(u0);
RWStructuredBuffer<ParticleDrawData>	drawList	: register(u1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	drawArgs[0] = drawList.IncrementCounter() * 6;
	drawArgs[1] = 1;
	drawArgs[2] = 1;
	drawArgs[3] = 1;
	drawArgs[4] = 0;

}