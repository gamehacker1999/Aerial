#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

//struct to define a particle
struct Particle
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT3 startVelocity;
	XMFLOAT3 startPos;
	float size;
	float age;
	float rotStart;
	float rotEnd;
	float rot;
};

//single vertex of a particle
//each particle needs four vertcies to make a quad
struct ParticleVertex 
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};