#pragma once
//class to manage a specific group of particles
#include"Particles.h"
#include"SimpleShader.h"
#include<memory>
#include"Camera.h"
#include<random>
#include<vector>

#define MAX_PARTICLES 250
class Emitter
{
public:
	Emitter(
		int maxParticles,
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
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture
	);
	~Emitter();

	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3 pos);
	void SetAcceleration(XMFLOAT3 acel);

	void UpdateParticles(float deltaTime);
	void Draw(ID3D11DeviceContext* context, std::shared_ptr<Camera> camera);

	int particlesPerSecond;
	float secondsPerParticle;
	float timeSinceEmit;

	int livingParticleCount;
	float lifetime;

	XMFLOAT3 emitterAcceleration;
	XMFLOAT3 emitterPosition;
	XMFLOAT3 startVelocity;

	XMFLOAT3 positionRandomRange;
	XMFLOAT3 velocityRandomRange;
	XMFLOAT4 rotationRandomRanges; // Min start, max start, min end, max end

	XMFLOAT4 startColor;
	XMFLOAT4 endColor;
	float startSize;
	float endSize;

	// Particle array
	Particle* particles;
	int maxParticles;
	int firstDeadIndex;
	int firstAliveIndex;

	// Rendering
	ParticleVertex* particleVertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* texture;
	SimpleVertexShader* vs;
	SimplePixelShader* ps;

	// Update Methods
	void UpdateSingleParticle(float dt, int index);
	void SpawnParticle();

	// Copy methods
	void CopyParticlesToGPU(ID3D11DeviceContext* context, std::shared_ptr<Camera> camera);
	void CopyOneParticle(int index, std::shared_ptr<Camera> camera);
	XMFLOAT3 CalcParticleVertexPosition(int particleIndex, int quadCornerIndex, std::shared_ptr<Camera> camera);
};

