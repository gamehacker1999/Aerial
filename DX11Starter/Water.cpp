#include "Water.h"

Water::Water(std::shared_ptr<Mesh> waterMesh, ID3D11ShaderResourceView* waterTex,
	ID3D11ShaderResourceView* waterNormal1, ID3D11ShaderResourceView* waterNormal2,
	SimplePixelShader* waterPS, SimpleVertexShader* waterVS, ID3D11SamplerState* samplerState)
{
	this->waterMesh = waterMesh;
	this->waterTex = waterTex;
	this->waterNormal1 = waterNormal1;
	this->waterNormal2 = waterNormal2;
	this->waterPS = waterPS;
	this->waterVS = waterVS;
	this->samplerState = samplerState;
}

Water::~Water()
{
}

void Water::Update(float deltaTime,XMFLOAT3 shipPos)
{
	//setting the world matrix for water
	XMFLOAT3 curPos = shipPos;
	curPos.y = -10;
	curPos.z += 30;
	curPos.x = 0;
	XMFLOAT3 scale = XMFLOAT3(10.f, 10.f, 10.f);
	XMMATRIX matTrans = XMMatrixTranslationFromVector(XMLoadFloat3(&curPos));
	XMMATRIX matScale = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
	XMMATRIX rot = XMMatrixRotationQuaternion(XMQuaternionIdentity());

	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(matScale* rot* matTrans));
}

void Water::Draw(Light lights, ID3D11ShaderResourceView* cubeMap, std::shared_ptr<Camera> camera,
	ID3D11DeviceContext* context, float deltaTime)
{
	waterVS->SetMatrix4x4("world", worldMat);
	waterVS->SetMatrix4x4("view", camera->GetViewMatrix());
	waterVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	waterVS->CopyAllBufferData();
	waterVS->SetShader();

	static float scrollX = 0;
	static float scrollY = 0;

	scrollX += 0.07f*deltaTime;
	scrollY += 0.07f*deltaTime;

	waterPS->SetFloat("scrollX", scrollX);
	waterPS->SetFloat("scrollY", scrollY);
	waterPS->SetData("dirLight", &lights, sizeof(Light));
	waterPS->SetFloat3("cameraPos", camera->GetPosition());
	waterPS->SetShaderResourceView("waterTexture", waterTex);
	waterPS->SetShaderResourceView("normalTexture1", waterNormal1);
	waterPS->SetShaderResourceView("normalTexture2", waterNormal2);
	waterPS->SetSamplerState("sampleOptions", samplerState);
	waterPS->CopyAllBufferData();
	waterPS->SetShader();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	auto tempVertBuffer = waterMesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(waterMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(waterMesh->GetIndexCount(), 0, 0);
}
