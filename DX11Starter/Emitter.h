#pragma once
//class to manage a specific group of particles
#include"Particles.h"
#include"SimpleShader.h"
#include<memory>
#include"Camera.h"
#include<wrl\client.h>
#include<random>
#include<vector>

#define MAX_PARTICLES 250
using namespace Microsoft::WRL;
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
		SimpleComputeShader* deadListInitCS,
		SimpleComputeShader* updateCS,
		SimpleComputeShader* emitCS,
		SimpleComputeShader* drawCopyCS,
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture
	);
	~Emitter();

	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3 pos);
	void SetAcceleration(XMFLOAT3 acel);

	void UpdateParticles(float deltaTime, float currentTime, ID3D11DeviceContext* context);
	void Draw(ID3D11DeviceContext* context, XMFLOAT4X4 view, XMFLOAT4X4 projection,float currentTime);

	void SetTemporary(float emitterLife);
	bool IsDead();
	void Explosive();

private:

	int particlesPerSecond;
	float secondsPerParticle;
	float timeSinceEmit;
	bool isTemp; //is the emitter temporary
	float emitterLifetime;
	float emitterAge;
	bool isDead;
	bool explosive;

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
	SimpleComputeShader* deadListInitCS;
	SimpleComputeShader* updateCS;
	SimpleComputeShader* copyDrawCS;
	SimpleComputeShader* emitCS;

	ID3D11Buffer* particleBuffer;
	ID3D11ShaderResourceView* particleData;
	ComPtr<ID3D11UnorderedAccessView> particlePoolUAV;
	ComPtr<ID3D11UnorderedAccessView> particleDeadUAV;
	ComPtr <ID3D11UnorderedAccessView> particleDrawUAV;
	ComPtr <ID3D11ShaderResourceView> particleDrawSRV;
	ComPtr <ID3D11UnorderedAccessView> drawArgsUAV;
	// Indirect draw buffer
	ComPtr<ID3D11Buffer> drawArgsBuffer;
	

	// Update Methods
	void UpdateSingleParticle(float dt, int index, float currentTime);
	void SpawnParticle(float currentTime);
};

