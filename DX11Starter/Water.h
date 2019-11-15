#pragma once
#include"Mesh.h"
#include<d3d11.h>
#include<DirectXMath.h>
#include"SimpleShader.h"
#include"Camera.h"
#include "Lights.h"
#include<memory>
using namespace DirectX;
class Water
{
	//water variables
	std::shared_ptr<Mesh> waterMesh;
	ID3D11ShaderResourceView* waterTex;
	ID3D11ShaderResourceView* waterNormal1;
	ID3D11ShaderResourceView* waterNormal2;

	SimplePixelShader* waterPS;
	SimpleVertexShader* waterVS;
	ID3D11SamplerState* samplerState;

	XMFLOAT4X4 worldMat;

public:
	Water(std::shared_ptr<Mesh> waterMesh, ID3D11ShaderResourceView* waterTex,
		ID3D11ShaderResourceView* waterNormal1, ID3D11ShaderResourceView* waterNormal2,
		SimplePixelShader* waterPS, SimpleVertexShader* waterVS, ID3D11SamplerState* samplerState);
	~Water();

	void Update(float deltaTime, XMFLOAT3 shipPos);

	void Draw(Light lights, ID3D11ShaderResourceView* cubeMap,std::shared_ptr<Camera> camera, 
		ID3D11DeviceContext* context, float deltaTime);
};

