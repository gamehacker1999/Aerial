#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include"Mesh.h"
#include"Ship.h"
#include "Bullet.h"
#include<vector>
#include"Camera.h"
#include"Lights.h"
#include"Skybox.h"
#include"Textures.h"
#include"Terrain.h"

#define MAX_BULLETS 3
class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();
	void CreateIrradianceMaps();
	void CreatePrefilteredMaps();
	void CreateEnvironmentLUTs();

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	//pbr pixel shader
	SimplePixelShader* pbrPixelShader;

	//vertexshader for shadows
	SimpleVertexShader* shadowVertexShader;
	SimplePixelShader* shadowPixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//3 meshes to draw on the scene
	std::shared_ptr<Mesh> mesh1;
	std::shared_ptr<Mesh> mesh2;
	std::shared_ptr<Mesh> mesh3;

	//sampler state for basic textures
	ID3D11SamplerState* samplerState;

	//creating a list of vectors
	std::shared_ptr<Ship> ship;
	std::vector<std::shared_ptr<Bullet>> bullets;
	std::vector<std::shared_ptr<Entity>> entities;

	//list of lights
	//std::vector<Light> lights;

	//camera
	std::shared_ptr<Camera> camera;

	//terrain
	std::shared_ptr<Terrain> terrain;

	//creating a directional light
	DirectionalLight directionalLight;
	DirectionalLight directionalLight2;

	//light struct array
	Light lights[MAX_LIGHTS];

	//creating a skybox and it's shaders
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;
	std::shared_ptr<Skybox> skybox;

	//depth stencil for skybox
	ID3D11DepthStencilState* dssLessEqual;

	//view and projection matrices for the irradiance map
	std::vector<XMFLOAT4X4> cubemapViews;
	XMFLOAT4X4 cubemapProj;
	ID3D11RasterizerState* skyRS;
	//textures, depth stencil, srv, and render target for irradiance map
	ID3D11Texture2D* irradianceMapTexture;
	ID3D11DepthStencilView* irradienceDepthStencil;
	ID3D11ShaderResourceView* irradienceSRV;
	ID3D11RenderTargetView* irradienceRTV[6];
	D3D11_VIEWPORT irradianceViewport;

	ID3D11RasterizerState* backCullRS;

	//texture, srv, and rtv for the prefiltered cubemap
	ID3D11Texture2D* prefileteredMapTexture;
	ID3D11ShaderResourceView* prefilteredSRV;
	ID3D11RenderTargetView* prefilteredRTV[6];

	//texture, rtv, and srv for environment look up texture
	ID3D11Texture2D* environmentBrdfTexture;
	ID3D11ShaderResourceView* environmentBrdfSRV;
	ID3D11RenderTargetView* environmentBrdfRTV;

	//vertex and pixel shader for irradience map, prefiltered map, and environment brdf
	SimpleVertexShader* irradianceVS;
	SimplePixelShader* irradiancePS;
	SimplePixelShader* prefilteredMapPS;
	SimplePixelShader* integrationBRDFPS;

	//simple shader to render a full screen quad
	SimpleVertexShader* fullScreenTriangleVS;

	//1d texture color band
	ID3D11ShaderResourceView* celShadingSRV;

	//creating blend states
	ID3D11BlendState* blendState;

	//I'm being dumb shhhhhh
	bool fired = false;
};

