#include "Game.h"
#include "Vertex.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	shadowVertexShader = nullptr;
	shadowPixelShader = nullptr;

	fullScreenTrianglePS = nullptr;
	pbrPixelShader = nullptr;

	cubemapViews.reserve(6); //6 view matrices for 6 faces of the cube

	irradianceMapTexture = nullptr;
	irradienceDepthStencil = nullptr;
	irradienceSRV = nullptr;
	irradiancePS = nullptr;
	irradianceVS = nullptr;

	prefilteredSRV = nullptr;
	prefileteredMapTexture = nullptr;
	prefilteredMapPS = nullptr;

	integrationBRDFPS = nullptr;

	fullScreenTriangleVS = nullptr;

	celShadingSRV = nullptr;

	terrain = nullptr;

	blendState = nullptr;

	backCullRS = nullptr;

	skyRS = nullptr;

	waterDiffuse = nullptr;
	waterNormal1 = nullptr;
	waterNormal2 = nullptr;
	waterPS = nullptr;
	waterVS = nullptr;
	waterReflectionRTV = nullptr;
	waterReflectionSRV = nullptr;
	waterReflectionPS = nullptr;
	waterReflectionVS = nullptr;
	waterSampler = nullptr;

	terrainTexture1 = nullptr;
	terrainTexture2 = nullptr;
	terrainTexture3 = nullptr;
	terrainNormalTexture1  = nullptr;
	terrainNormalTexture2  = nullptr;
	terrainNormalTexture3  = nullptr;
	terrainBlendMap=nullptr;
	terrainPS = nullptr;

	noiseR1 = nullptr;
	noiseI1 = nullptr;
	noiseR2 = nullptr;
	noiseI2 = nullptr;
	foam = nullptr;
	twiddleFactorsCS = nullptr;
	butterflyCS = nullptr;
	inversionCS = nullptr;
	sobelFilter = nullptr;
	jacobianCS = nullptr;

	prevMousePos = { 0,0 };	

	ppSampler = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	
	if (shadowVertexShader)
		delete shadowVertexShader;

	if (shadowPixelShader)
		delete shadowPixelShader;

	if (pbrPixelShader)
		delete pbrPixelShader;

	if (pbrRimLightingShader)
		delete pbrRimLightingShader;

	if (samplerState)
		samplerState->Release();

	if(ppSampler)
		ppSampler->Release();

	if (irradianceMapTexture)
		irradianceMapTexture->Release();

	if (irradienceDepthStencil)
		irradienceDepthStencil->Release();

	if (irradienceSRV)
		irradienceSRV->Release();

	//releasing the render targets
	for (size_t i = 0; i < 6; i++)
	{
		if (irradienceRTV[i])
			irradienceRTV[i]->Release();
	}

	if (prefilteredSRV)
		prefilteredSRV->Release();

	if (ppRTV)
		ppRTV->Release();
	
	if (ppSRV)
		ppSRV->Release();

	if (ppVS)
		delete ppVS;
	
	if (ppPS)
		delete ppPS;

	if (prefileteredMapTexture)
		prefileteredMapTexture->Release();

	if (irradiancePS)
		delete irradiancePS;

	if (irradianceVS)
		delete irradianceVS;

	if (prefilteredMapPS)
		delete prefilteredMapPS;

	if (integrationBRDFPS)
		delete integrationBRDFPS;

	if (fullScreenTriangleVS)
		delete fullScreenTriangleVS;

	if (environmentBrdfSRV)
		environmentBrdfSRV->Release();

	if (celShadingSRV)
		celShadingSRV->Release();

	if (particleBlendState)
		particleBlendState->Release();

	if (blendState)
		blendState->Release();

	if (backCullRS)
		backCullRS->Release();

	if (skyRS)
		skyRS->Release();

	if (particlePS)
		delete particlePS;

	if (particleVS)
		delete particleVS;

	if (particleTexture)
		particleTexture->Release();

	if (waterPS)
		delete waterPS;

	if (waterVS)
		delete waterVS;

	delete h0CS;
	delete htCS;
	delete twiddleFactorsCS;
	delete butterflyCS;
	delete inversionCS;
	delete sobelFilter;
	delete jacobianCS;

	if (foam)
		foam->Release();

	if (waterDiffuse)
		waterDiffuse->Release();

	if (waterNormal1)
		waterNormal1->Release();

	if (waterNormal2)
		waterNormal2->Release();

	if (waterReflectionPS)
		delete waterReflectionPS;

	if (waterReflectionVS)
		delete waterReflectionVS;

	if (waterReflectionRTV)
		waterReflectionRTV->Release();

	if (waterReflectionSRV)
		waterReflectionSRV->Release();

	if (noiseR1)
		noiseR1->Release();

	if (noiseI1)
		noiseI1->Release();

	if (noiseR2)
		noiseR2->Release();

	if (noiseI2)
		noiseI2->Release();

	if (waterSampler)
		waterSampler->Release();

	if (fullScreenTrianglePS)
		delete fullScreenTrianglePS;

	if (shadowDepthStencil) shadowDepthStencil->Release();
	if (shadowMapTexture) shadowMapTexture->Release();
	if (shadowRasterizerState) { shadowRasterizerState->Release(); }
	if (shadowSamplerState) { shadowSamplerState->Release(); }
	if (shadowSRV) shadowSRV->Release();


	textureSRV->Release();

	if (terrainPS)
		delete terrainPS;
	//trying to load a texture

	//trying to load a normalMap
	normalTextureSRV->Release();

	roughnessTextureSRV->Release();

	metalnessTextureSRV->Release();

	goldTextureSRV->Release();
	//trying to load a texture

	//trying to load a normalMap
	goldNormalTextureSRV->Release();

	goldRoughnessTextureSRV->Release();

	goldMetalnessTextureSRV->Release();

	if (terrainTexture1)
		terrainTexture1->Release();

	if (terrainTexture2)
		terrainTexture2->Release();

	if (terrainTexture3)
		terrainTexture3->Release();

	if (terrainNormalTexture1)
		terrainNormalTexture1->Release();

	if (terrainNormalTexture2)
		terrainNormalTexture2->Release();

	if (terrainNormalTexture3)
		terrainNormalTexture3->Release();
	if(buildingTextureSRV) buildingTextureSRV->Release();
	if(buildingNormalTextureSRV) buildingNormalTextureSRV->Release();
	if(buildingRoughnessTextureSRV) buildingRoughnessTextureSRV->Release();
	if(buildingMetalnessTextureSRV) buildingMetalnessTextureSRV->Release();
	if(boulderTextureSRV) boulderTextureSRV->Release();

	if (terrainBlendMap)
		terrainBlendMap->Release();


	//releasing depth stencil
	dssLessEqual->Release();

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	GenerateTerrain();

	//initalizing camera
	camera = std::make_shared<Camera>(XMFLOAT3(0.0f, 3.5f, -18.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	camera->CreateProjectionMatrix((float)width / height); //creating the camera projection matrix

	//specifying the directional light
	directionalLight.ambientColor = XMFLOAT4(0.3f, 0.3f ,0.3f,1.f);
	directionalLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

	//second light
	directionalLight2.ambientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight2.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight2.direction = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	XMFLOAT3 worldOrigin = XMFLOAT3(0.f, 0.f, 0.f);
	lights[0] = {};
	lights[0].type = LIGHT_TYPE_DIR;
	lights[0].direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	lights[0].diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 lightDir1Pos;
	XMStoreFloat3(&lightDir1Pos, XMLoadFloat3(&worldOrigin) - XMLoadFloat3(&lights[0].direction) * 100);
	lights[0].position = lightDir1Pos;
	lights[0].range = 10.f;

	//setting depth stencil for skybox;
		//depth stencil state for skybox
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	memset(&dssDesc, 0, sizeof(dssDesc));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dssDesc, &dssLessEqual);

	//creating a blend state
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, &blendState);

	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Still respect pixel shader output alpha
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepth);

	//description of the shadow mapping depth buffer
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Width = 1024;
	shadowMapDesc.Height = 1024;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;

	//creating a texture
	device->CreateTexture2D(&shadowMapDesc, nullptr, &shadowMapTexture);

	//description for depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(shadowMapTexture, &depthStencilViewDesc, &shadowDepthStencil);

	//creating a shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	ZeroMemory(&shadowSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(shadowMapTexture, &shadowSRVDesc, &shadowSRV);

	//setting up the shadow viewport
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.Width = 1024.0f;
	shadowViewport.Height = 1024.0f;
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;
	shadowViewport.TopLeftX = 0.0f;
	shadowViewport.TopLeftY = 0.0f;

	//rasterizer for pixel shader
	D3D11_RASTERIZER_DESC shadowRasterizerDesc;
	ZeroMemory(&shadowRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_BACK;
	shadowRasterizerDesc.DepthClipEnable = true;
	shadowRasterizerDesc.FrontCounterClockwise = false;
	shadowRasterizerDesc.DepthBias = 1000;
	shadowRasterizerDesc.DepthBiasClamp = 0.0f;
	shadowRasterizerDesc.SlopeScaledDepthBias = 1.0f;

	//creating this rasterizer
	device->CreateRasterizerState(&shadowRasterizerDesc, &shadowRasterizerState);

	//sampler for the shadow texture
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	memset(&shadowSamplerDesc, 0, sizeof(shadowSamplerDesc));
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

	device->CreateSamplerState(&shadowSamplerDesc, &shadowSamplerState);


	// Create post process resources -----------------------------------------
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &ppRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &ppSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();


	//creating water sampler, srv and rtv
	D3D11_SAMPLER_DESC waterSampDesc = {};
	waterSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	waterSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	waterSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	waterSampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	waterSampDesc.MaxAnisotropy = 4;
	waterSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&waterSampDesc, &waterSampler);

	// Create a post-process sampler state
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_ANISOTROPIC;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxAnisotropy = 16;
	ppSampDesc.MinLOD = 0;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&ppSampDesc, &ppSampler);

	ID3D11Texture2D* waterReflectionTexture2D=nullptr;

	D3D11_TEXTURE2D_DESC waterReflectionTexDesc = {};
	waterReflectionTexDesc.Width = width;
	waterReflectionTexDesc.Height = height;
	waterReflectionTexDesc.MipLevels = 1;
	waterReflectionTexDesc.ArraySize = 1;
	waterReflectionTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	waterReflectionTexDesc.Usage = D3D11_USAGE_DEFAULT;
	waterReflectionTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	waterReflectionTexDesc.CPUAccessFlags = 0;
	waterReflectionTexDesc.MipLevels = 0;
	waterReflectionTexDesc.SampleDesc.Quality = 0;
	waterReflectionTexDesc.SampleDesc.Count = 1;
	device->CreateTexture2D(&waterReflectionTexDesc, 0, &waterReflectionTexture2D);

	D3D11_RENDER_TARGET_VIEW_DESC waterReflectionRTVDesc = {};
	waterReflectionRTVDesc.Format = waterReflectionTexDesc.Format;
	waterReflectionRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	waterReflectionRTVDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(waterReflectionTexture2D, &waterReflectionRTVDesc, &waterReflectionRTV);

	D3D11_SHADER_RESOURCE_VIEW_DESC waterReflectionSRVDesc = {};
	waterReflectionSRVDesc.Format = waterReflectionTexDesc.Format;
	waterReflectionSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	waterReflectionSRVDesc.Texture2D.MipLevels = 1;
	waterReflectionSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(waterReflectionTexture2D, &waterReflectionSRVDesc, &waterReflectionSRV);

	waterReflectionTexture2D->Release();

	//terrain = std::make_shared<Terrain>();
	//terrain->LoadHeightMap(device,"../../Assets/Textures/terrain.raw" );

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CreateIrradianceMaps();

	CreatePrefilteredMaps();

	CreateEnvironmentLUTs();

	InitializeEntities();

	bulletCounter = 0;
	buildingTimer = rand() % 30 + 45;
	buildingCounter = 0;

	shipGas = std::make_shared<Emitter>(
		300, //max particles
		50, //particles per second
		0.4f, //lifetime
		0.6f, //start size
		0.03f, //end size
		XMFLOAT4(1, 1.0f, 1.0f, 1.0f), //start color
		XMFLOAT4(1, 0.1f, 0.1f, 0.6f), //end color
		XMFLOAT3(0, 0, -1.f), //start vel
		XMFLOAT3(0.2f, 0.2f, 0.2f), //velocity deviation range
		ship->GetPosition(), //start position
		XMFLOAT3(0.1f, 0.1f, 0.1f), //position deviation range
		XMFLOAT4(-2, 2, -2, 2), //rotation around z axis
		XMFLOAT3(0.f, -1.f, 0.f), //acceleration
		device, particleVS, particlePS, particleTexture);

	emitterList.emplace_back(shipGas);

	//look up for what each piece means
	shipGas2 = std::make_shared<Emitter>(
		300, 
		50, 
		0.4f, 
		0.6f, 
		0.03f, 
		XMFLOAT4(1, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1, 0.1f, 0.1f, 0.6f), 
		XMFLOAT3(0, 0, -1.f), 
		XMFLOAT3(0.2f, 0.2f, 0.2f),
		ship->GetPosition(), 
		XMFLOAT3(0.1f, 0.1f, 0.1f), 
		XMFLOAT4(-2, 2, -2, 2),
		XMFLOAT3(0.f, -1.f, 0.f), 
		device, particleVS, particlePS, particleTexture);

	emitterList.emplace_back(shipGas2);

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{

	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	shadowVertexShader = new SimpleVertexShader(device, context);
	shadowVertexShader->LoadShaderFile(L"ShadowsVS.cso");

	shadowPixelShader = new SimplePixelShader(device, context);
	shadowPixelShader->LoadShaderFile(L"ShadowsPS.cso");

	pbrPixelShader = new SimplePixelShader(device, context);
	pbrPixelShader->LoadShaderFile(L"PBRPixelShader.cso");

	irradiancePS = new SimplePixelShader(device, context);
	irradiancePS->LoadShaderFile(L"IrradianceMapPS.cso");

	irradianceVS = new SimpleVertexShader(device, context);
	irradianceVS->LoadShaderFile(L"IrradianceMapVS.cso");

	prefilteredMapPS = new SimplePixelShader(device, context);
	prefilteredMapPS->LoadShaderFile(L"PrefilteredMapPS.cso");

	integrationBRDFPS = new SimplePixelShader(device, context);
	integrationBRDFPS->LoadShaderFile(L"IntegrationBRDFPixelShader.cso");

	fullScreenTriangleVS = new SimpleVertexShader(device, context);
	fullScreenTriangleVS->LoadShaderFile(L"FullScreenTriangleVS.cso");

	waterPS = new SimplePixelShader(device, context);
	waterPS->LoadShaderFile(L"WaterPS.cso");

	waterVS = new SimpleVertexShader(device, context);
	waterVS->LoadShaderFile(L"WaterVS.cso");

	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlesPS.cso");

	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticlesVS.cso");

	waterPS = new SimplePixelShader(device, context);
	waterPS->LoadShaderFile(L"WaterPS.cso");

	waterReflectionPS = new SimplePixelShader(device, context);
	waterReflectionPS->LoadShaderFile(L"WaterReflectionPS.cso");

	waterReflectionVS = new SimpleVertexShader(device, context);
	waterReflectionVS->LoadShaderFile(L"WaterReflectionVS.cso");

	fullScreenTrianglePS = new SimplePixelShader(device, context);
	fullScreenTrianglePS->LoadShaderFile(L"FullScreenTrianglePS.cso");

	pbrRimLightingShader = new SimplePixelShader(device, context);
	pbrRimLightingShader->LoadShaderFile(L"PBRRimLighting.cso");

	terrainPS = new SimplePixelShader(device, context);
	terrainPS->LoadShaderFile(L"TerrainPS.cso");

	h0CS = new SimpleComputeShader(device, context);
	h0CS->LoadShaderFile(L"H0OceanCS.cso");

	htCS = new SimpleComputeShader(device, context);
	htCS->LoadShaderFile(L"HtOceanCS.cso");

	twiddleFactorsCS = new SimpleComputeShader(device, context);
	twiddleFactorsCS->LoadShaderFile(L"TwiddleFactorsCS.cso");

	butterflyCS = new SimpleComputeShader(device, context);
	butterflyCS->LoadShaderFile(L"ButterflyCS.cso");

	inversionCS = new SimpleComputeShader(device, context);
	inversionCS->LoadShaderFile(L"InversionCS.cso");

	sobelFilter = new SimpleComputeShader(device, context);
	sobelFilter->LoadShaderFile(L"NormalMapCS.cso");

	jacobianCS = new SimpleComputeShader(device, context);
	jacobianCS->LoadShaderFile(L"JacobianCS.cso");

	// Post process shaders
	ppVS = new SimpleVertexShader(device, context);
	ppVS->LoadShaderFile(L"PostProcessVS.cso");

	ppPS = new SimplePixelShader(device, context);
	ppPS->LoadShaderFile(L"PostProcessPS.cso");
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

	//adding three entities with the meshes
	entities.reserve(100);

	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipDiffuse.jpg",0,&textureSRV);

	//trying to load a normalMap
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipNormal.jpg", 0, &normalTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipRoughness.jpg", 0, &roughnessTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipMetallic.jpg", 0, &metalnessTextureSRV);

	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeDiffuse.png", 0, &goldTextureSRV);

	//trying to load a normalMap
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeNormal.png", 0, &goldNormalTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeRoughness.png", 0, &goldRoughnessTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeMetallic.png", 0, &goldMetalnessTextureSRV);

	//Load Building textures
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/building.png", 0, &buildingTextureSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/buildingNormal.png", 0, &buildingNormalTextureSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/buildingRoughness.png", 0, &buildingRoughnessTextureSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/buildingMetal.png", 0, &buildingMetalnessTextureSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/boulder.png", 0, &boulderTextureSRV);


	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterDiffuse.jpg", 0, &waterDiffuse);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/particle.jpg", 0, &particleTexture);

	//loading cel shading
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/ColorBand.jpg",0,&celShadingSRV);

	//loading water textures
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water1Normal.jpg", 0, &waterNormal1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/water2Normal.jpg", 0, &waterNormal2);

	//loading terrain textures
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/valley_splat.png", 0, &terrainBlendMap);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/snow.jpg", 0, &terrainTexture1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/grass3.png", 0, &terrainTexture2);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/mountain3.png", 0, &terrainTexture3);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/snow_normals.jpg", 0, &terrainNormalTexture1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/grass3_normals.png", 0, &terrainNormalTexture2);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/mountain3_normals.png", 0, &terrainNormalTexture3);

	//noise textures
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/NoiseR1.jpg", 0, &noiseR1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/NoiseI1.jpg", 0, &noiseI1);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/NoiseR2.jpg", 0, &noiseR2);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/NoiseI2.jpg", 0, &noiseI2);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/foam.png",	0, &foam);

	//creating a sampler state
	//sampler state description
	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerState); //creating the sampler state
	
	//creating a material for these entities

	//also used for obstacles
	material = std::make_shared<Material>(vertexShader, pbrPixelShader,samplerState,
		textureSRV, normalTextureSRV,roughnessTextureSRV,metalnessTextureSRV);

	std::shared_ptr<Material> goldMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, samplerState,
		goldTextureSRV, goldNormalTextureSRV, goldRoughnessTextureSRV, goldMetalnessTextureSRV);

	obstacleMat = std::make_shared<Material>(vertexShader, pbrRimLightingShader, samplerState,
		goldTextureSRV, goldNormalTextureSRV, goldRoughnessTextureSRV, goldMetalnessTextureSRV);

	buildingMat = std::make_shared<Material>(vertexShader, pbrPixelShader, samplerState,
		buildingTextureSRV, buildingNormalTextureSRV, buildingRoughnessTextureSRV, buildingMetalnessTextureSRV);
	boulderMat = std::make_shared<Material>(vertexShader, pbrPixelShader, samplerState,
		boulderTextureSRV, boulderTextureSRV, boulderTextureSRV, boulderTextureSRV);

	shipMesh = std::make_shared<Mesh>("../../Assets/Models/ship.obj",device);
	std::shared_ptr<Mesh> object = std::make_shared<Mesh>("../../Assets/Models/cube.obj", device);
	obstacleMesh = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device);
	bulletMesh = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device);
	buildingMesh = std::make_shared<Mesh>("../../Assets/Models/building.obj", device);
	boulderMesh = std::make_shared<Mesh>("../../Assets/Models/boulder.obj", device);
	waterMesh = std::make_shared<Mesh>("../../Assets/Models/quad.obj", device);

	ID3D11SamplerState* samplerStateCube;
	//sampler state description
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerStateCube); //creating the sampler state

	skybox = std::make_shared<Skybox>();
	//creating skybox
	skybox->LoadSkybox(L"../../Assets/Textures/SunnyCubeMap.dds", device, context,samplerStateCube);

	D3D11_RASTERIZER_DESC skyRSDesc = {};
	skyRSDesc.FillMode = D3D11_FILL_SOLID;
	skyRSDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&skyRSDesc, &skyRS);
}

void Game::GenerateTerrain()
{
	terrain = std::make_shared<Terrain>(
		device, //device
		"../../Assets/Textures/valley.raw16",//file
		513, //width
		513, //height
		TerrainBitDepth::BitDepth_16, //bit depth
		40.0f, //yscale
		0.2f, //xz scale
		1.0f, //uv scale
		terrainTexture1,
		terrainTexture2,
		terrainTexture3,
		terrainBlendMap,
		terrainNormalTexture1,
		terrainNormalTexture2,
		terrainNormalTexture3,
		samplerState,
		vertexShader,
		terrainPS
	);
}

void Game::InitializeEntities()
{
	ship = std::make_shared<Ship>(shipMesh, material);

	// set ship scale to smaller
	float shipScale = 0.5f;
	ship->SetScale({ shipScale, shipScale, shipScale });

	ship->UseRigidBody();
	ship->SetTag("Player");
	entities.emplace_back(ship);

	auto shipOrientation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), 3.14159f);
	XMFLOAT4 retShipRotation;
	XMStoreFloat4(&retShipRotation, shipOrientation);
	ship->SetRotation(retShipRotation);
	ship->SetOriginalRotation(retShipRotation);

	// set ship speed
	ship->SetSpeed(5);

	for (size_t i = 0; i < MAX_BULLETS; i++)
	{
		std::shared_ptr<Bullet> newBullet = std::make_shared<Bullet>(bulletMesh, material);
		newBullet->UseRigidBody();
		//newBullet->SetScale(XMFLOAT3(0.3f, 0.3f, 0.3f));
		bullets.emplace_back(newBullet);
	}

	water = std::make_shared<Water>(waterMesh, 
		waterDiffuse, 
		waterNormal1, waterNormal2, 
		waterPS, waterVS,h0CS, htCS, twiddleFactorsCS, butterflyCS, inversionCS, sobelFilter, jacobianCS,
		samplerState,device,
		noiseR1,noiseI1,noiseR2,noiseI2);

	water->CreateH0Texture();

	water->CreateTwiddleIndices();
}

void Game::CreateIrradianceMaps()
{
	XMFLOAT4X4 cubePosxView;
	XMFLOAT4X4 cubeNegxView;
	XMFLOAT4X4 cubePoszView;
	XMFLOAT4X4 cubeNegzView;
	XMFLOAT4X4 cubePosyView;
	XMFLOAT4X4 cubeNegyView;

	//postive x face of cube
	XMStoreFloat4x4(&cubePosxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosxView);

	//negative x face of cube
	XMStoreFloat4x4(&cubeNegxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegxView);

	//postive y face of cube
	XMStoreFloat4x4(&cubePosyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosyView);

	//negative y face of cube
	XMStoreFloat4x4(&cubeNegyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegyView);

	//postive z face of cube
	XMStoreFloat4x4(&cubePoszView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePoszView);

	//negative z face of cube
	XMStoreFloat4x4(&cubeNegzView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegzView);

	//setting the cubemap projection
	XMStoreFloat4x4(&cubemapProj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));

	//generating the irradience map
	D3D11_TEXTURE2D_DESC irradienceTexDesc;
	//ZeroMemory(&irradienceTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	irradienceTexDesc.ArraySize = 6;
	irradienceTexDesc.MipLevels = 1;
	irradienceTexDesc.Width = 64;
	irradienceTexDesc.Height = 64;
	irradienceTexDesc.CPUAccessFlags = 0;
	irradienceTexDesc.SampleDesc.Count = 1;
	irradienceTexDesc.SampleDesc.Quality = 0;
	irradienceTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	irradienceTexDesc.Usage = D3D11_USAGE_DEFAULT;
	irradienceTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	irradienceTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	device->CreateTexture2D(&irradienceTexDesc, 0, &irradianceMapTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC irradienceSRVDesc;
	ZeroMemory(&irradienceSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	irradienceSRVDesc.Format = irradienceTexDesc.Format;
	irradienceSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	irradienceSRVDesc.TextureCube.MipLevels = 1;
	irradienceSRVDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(irradianceMapTexture, &irradienceSRVDesc, &irradienceSRV);

	D3D11_RENDER_TARGET_VIEW_DESC irradianceRTVDesc;
	ZeroMemory(&irradianceRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	//irradianceRTVDesc
	irradianceRTVDesc.Format = irradienceTexDesc.Format;
	irradianceRTVDesc.Texture2DArray.ArraySize = 1;
	irradianceRTVDesc.Texture2DArray.MipSlice = 0;
	irradianceRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;

	//creating viewport
	ZeroMemory(&irradianceViewport, sizeof(D3D11_VIEWPORT));
	irradianceViewport.Width = (float)64;
	irradianceViewport.Height = (float)64;
	irradianceViewport.MaxDepth = 1.0f;
	irradianceViewport.MinDepth = 0.0f;
	irradianceViewport.TopLeftX = 0.0f;
	irradianceViewport.TopLeftY = 0.0f;

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (UINT i = 0; i < 6; i++)
	{
		irradianceRTVDesc.Texture2DArray.FirstArraySlice = i;
		device->CreateRenderTargetView(irradianceMapTexture, &irradianceRTVDesc, &irradienceRTV[i]);	

		context->OMSetRenderTargets(1, &irradienceRTV[i], 0);
		context->RSSetViewports(1, &irradianceViewport);
		context->ClearRenderTargetView(irradienceRTV[i], color);

		irradianceVS->SetMatrix4x4("view", cubemapViews[i]);
		irradianceVS->SetMatrix4x4("projection", cubemapProj);
		irradiancePS->SetShaderResourceView("skybox", skybox->GetSkyboxTexture());
		irradiancePS->SetSamplerState("basicSampler", samplerState);

		irradiancePS->CopyAllBufferData();
		irradianceVS->CopyAllBufferData();

		irradiancePS->SetShader();
		irradianceVS->SetShader();

		auto tempVertexBuffer = skybox->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->OMSetDepthStencilState(dssLessEqual, 0);
		context->RSSetState(skyRS);
		context->DrawIndexed(skybox->GetIndexCount(), 0, 0);


	}

	
}

void Game::CreatePrefilteredMaps()
{

	XMFLOAT4X4 cubePosxView;
	XMFLOAT4X4 cubeNegxView;
	XMFLOAT4X4 cubePoszView;
	XMFLOAT4X4 cubeNegzView;
	XMFLOAT4X4 cubePosyView;
	XMFLOAT4X4 cubeNegyView;

	//postive x face of cube
	XMStoreFloat4x4(&cubePosxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosxView);

	//negative x face of cube
	XMStoreFloat4x4(&cubeNegxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegxView);

	//postive y face of cube
	XMStoreFloat4x4(&cubePosyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosyView);

	//negative y face of cube
	XMStoreFloat4x4(&cubeNegyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegyView);

	//postive z face of cube
	XMStoreFloat4x4(&cubePoszView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePoszView);

	//negative z face of cube
	XMStoreFloat4x4(&cubeNegzView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegzView);

	//setting the cubemap projection
	XMStoreFloat4x4(&cubemapProj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));

	//creating texture 2d for prefilted map
	D3D11_TEXTURE2D_DESC prefilteredTexDesc;
	UINT maxMipLevel = 5;
	prefilteredTexDesc.ArraySize = 6;
	prefilteredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	prefilteredTexDesc.CPUAccessFlags = 0;
	prefilteredTexDesc.Usage = D3D11_USAGE_DEFAULT;
	prefilteredTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	prefilteredTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	prefilteredTexDesc.MipLevels = maxMipLevel;
	prefilteredTexDesc.Width = 128;
	prefilteredTexDesc.Height = 128;
	prefilteredTexDesc.SampleDesc.Count = 1;
	prefilteredTexDesc.SampleDesc.Quality = 0;

	device->CreateTexture2D(&prefilteredTexDesc, 0, &prefileteredMapTexture);

	//creating shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC prefilteredSRVDesc;
	ZeroMemory(&prefilteredSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	prefilteredSRVDesc.Format = prefilteredTexDesc.Format;
	prefilteredSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	prefilteredSRVDesc.TextureCube.MipLevels = maxMipLevel;
	prefilteredSRVDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(prefileteredMapTexture, &prefilteredSRVDesc, &prefilteredSRV);

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (size_t mip = 0; mip < maxMipLevel; mip++)
	{
		//each mip is sqrt times smaller than previous one
		double width = 128 * std::pow(0.5f, mip);
		double height = 128 * std::pow(0.5f, mip);

		D3D11_RENDER_TARGET_VIEW_DESC prefilteredRTVDesc;
		ZeroMemory(&prefilteredRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		prefilteredRTVDesc.Format = prefilteredRTVDesc.Format;
		prefilteredRTVDesc.Texture2DArray.ArraySize = 1;
		prefilteredRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		prefilteredRTVDesc.Texture2DArray.MipSlice = (UINT)mip;

		D3D11_VIEWPORT currentMipViewport;
		ZeroMemory(&currentMipViewport, sizeof(currentMipViewport));
		currentMipViewport.Width = (float)width;
		currentMipViewport.Height = (float)height;
		currentMipViewport.MinDepth = 0.0f;
		currentMipViewport.MaxDepth = 1.0f;
		currentMipViewport.TopLeftX = 0.0f;
		currentMipViewport.TopLeftY = 0.0f;

		float roughness = float(mip) / float(maxMipLevel - 1);

		for (size_t i = 0; i < 6; i++)
		{
			prefilteredRTVDesc.Texture2DArray.FirstArraySlice = (UINT)i;
			device->CreateRenderTargetView(prefileteredMapTexture, &prefilteredRTVDesc, &prefilteredRTV[i]);

			context->OMSetRenderTargets(1, &prefilteredRTV[i], 0);
			context->RSSetViewports(1, &currentMipViewport);
			context->ClearRenderTargetView(prefilteredRTV[i], color);

			irradianceVS->SetMatrix4x4("view", cubemapViews[i]);
			irradianceVS->SetMatrix4x4("projection", cubemapProj);
			prefilteredMapPS->SetShaderResourceView("skybox", skybox->GetSkyboxTexture());
			prefilteredMapPS->SetSamplerState("basicSampler", samplerState);
			prefilteredMapPS->SetFloat("roughness", roughness);

			prefilteredMapPS->CopyAllBufferData();
			irradianceVS->CopyAllBufferData();

			prefilteredMapPS->SetShader();
			irradianceVS->SetShader();

			auto tempVertexBuffer = skybox->GetVertexBuffer();
			context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

			context->OMSetDepthStencilState(dssLessEqual, 0);
			context->RSSetState(skyRS);

			context->DrawIndexed(skybox->GetIndexCount(), 0, 0);

		}

		for (size_t i = 0; i < 6; i++)
		{
			if (prefilteredRTV[i])
				prefilteredRTV[i]->Release();
		}

	}
	
}

void Game::CreateEnvironmentLUTs()
{

	D3D11_TEXTURE2D_DESC integrationBrdfDesc;
	//ZeroMemory(&irradienceTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	integrationBrdfDesc.ArraySize = 1;
	integrationBrdfDesc.MipLevels = 0;
	integrationBrdfDesc.Width = 512;
	integrationBrdfDesc.Height = 512;
	integrationBrdfDesc.CPUAccessFlags = 0;
	integrationBrdfDesc.SampleDesc.Count = 1;
	integrationBrdfDesc.SampleDesc.Quality = 0;
	integrationBrdfDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	integrationBrdfDesc.Usage = D3D11_USAGE_DEFAULT;
	integrationBrdfDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	integrationBrdfDesc.MiscFlags = 0;

	device->CreateTexture2D(&integrationBrdfDesc, 0, &environmentBrdfTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC environmentBrdfSRVDesc;
	ZeroMemory(&environmentBrdfSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	environmentBrdfSRVDesc.Format = integrationBrdfDesc.Format;
	environmentBrdfSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	environmentBrdfSRVDesc.Texture2D.MipLevels = 1;
	environmentBrdfSRVDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(environmentBrdfTexture, &environmentBrdfSRVDesc, &environmentBrdfSRV);

	D3D11_RENDER_TARGET_VIEW_DESC enironmentBrdfRTVDesc;
	ZeroMemory(&enironmentBrdfRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	enironmentBrdfRTVDesc.Format = integrationBrdfDesc.Format;
	enironmentBrdfRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(environmentBrdfTexture, &enironmentBrdfRTVDesc, &environmentBrdfRTV);

	//creating a quad to render the LUT to
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>("../../Assets/Models/quad.obj", device);

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };
	context->OMSetDepthStencilState(NULL, 0);
	context->RSSetState(NULL);

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	D3D11_VIEWPORT integrationBrdfViewport;
	ZeroMemory(&integrationBrdfViewport, sizeof(D3D11_VIEWPORT));
	integrationBrdfViewport.Height = 512;
	integrationBrdfViewport.Width = 512;
	integrationBrdfViewport.MaxDepth = 1.0f;
	integrationBrdfViewport.MinDepth = 0.0f;
	integrationBrdfViewport.TopLeftX = 0.0f;
	integrationBrdfViewport.TopLeftY = 0.0f;

	context->RSSetViewports(1, &integrationBrdfViewport);
	context->OMSetRenderTargets(1, &environmentBrdfRTV, 0);
	context->ClearRenderTargetView(environmentBrdfRTV, color);

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	vertexShader->SetMatrix4x4("world", world);

	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	vertexShader->SetMatrix4x4("view", view);

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));
	vertexShader->SetMatrix4x4("projection", proj);

	//vertexShader->CopyAllBufferData();
	//vertexShader->SetShader();
	
	integrationBRDFPS->SetShader();
	fullScreenTriangleVS->SetShader();

	//auto tempVertBuffer = quad->GetVertexBuffer();
	//context->IASetVertexBuffers(0,1,&tempVertBuffer,&stride,&offset);
	//context->IASetIndexBuffer(quad->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);

	environmentBrdfRTV->Release();
	environmentBrdfTexture->Release();
}

void Game::RestartGame()
{

	//std::this_thread::sleep_for(std::chrono::seconds(2));

	camera->SetPositionTargetAndUp(XMFLOAT3(0.0f, 3.5f, -18.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i] = nullptr;
	}

	bulletCounter = 0;
	
	InitializeEntities();


}

void Game::DrawSceneOpaque(XMFLOAT4 clip)
{
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMFLOAT3 up(0.0f, 1.0f, 0.0f);

	//taking the center of the camera and backing up from the direction of the light
	//this is the position of the light
	//XMStoreFloat3(&directionLightPosition, XMLoadFloat3(&center) - XMLoadFloat3(&directionalLight.direction) * 10000.f);
	//creating the camera look to matrix
	auto tempLightView = XMMatrixLookAtLH(XMLoadFloat3(&lights[0].position),
		XMLoadFloat3(&ship->GetPosition()), XMLoadFloat3(&up));

	//storing the light view matrix
	XMFLOAT4X4 lightView;
	XMStoreFloat4x4(&lightView, XMMatrixTranspose(tempLightView));

	//calculating projection matrix
	XMFLOAT4X4 lightProjection;
	XMMATRIX tempLightProjection = XMMatrixOrthographicLH(100.f, 100.f,
		0.1f, 1000.0f);
	XMStoreFloat4x4(&lightProjection, XMMatrixTranspose(tempLightProjection));

	auto view = camera->GetViewMatrix();

	for (size_t i = 0; i < entities.size(); i++)
	{
		//preparing material for entity
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightView);
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", lightProjection);
		entities[i]->GetMaterial()->GetVertexShader()->SetFloat4("clipDistance", clip);
		entities[i]->PrepareMaterial(view, camera->GetProjectionMatrix());

		//adding lights and sending camera position
		entities[i]->GetMaterial()->GetPixelShader()->SetData("light", &directionalLight, sizeof(DirectionalLight)); //adding directional lights to the scene
		entities[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * MAX_LIGHTS);
		entities[i]->GetMaterial()->GetPixelShader()->SetInt("lightCount", 2);

		//entities[i]->GetMaterial()->GetPixelShader()->SetData("light2", &directionalLight2, sizeof(DirectionalLight));
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", camera->GetPosition());

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("cubeMap", skybox->GetSkyboxTexture());
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("celShading", celShadingSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("irradianceMap", irradienceSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("shadowMap", shadowSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("prefilteredMap", prefilteredSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("environmentBRDF", environmentBrdfSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetSamplerState("shadowSampler", shadowSamplerState);

		entities[i]->GetMaterial()->SetPixelShaderData();

		//setting the vertex and index buffer
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("shadowMap", nullptr);
	}
}

void Game::DrawSky(XMFLOAT4 clip)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->OMSetDepthStencilState(dssLessEqual, 0);

	auto view = camera->GetViewMatrix();

	/*if (reflect)
	{
		XMMATRIX reflectedMatrix = XMMatrixReflect(XMLoadFloat4(&clip));
		XMMATRIX tempView = XMMatrixMultiply(reflectedMatrix, XMLoadFloat4x4(&view));
		XMStoreFloat4x4(&view, tempView);
	}*/

	//draw the skybox
	context->RSSetState(skyRS);
	skybox->PrepareSkybox(view, camera->GetProjectionMatrix(), camera->GetPosition());
	auto tempVertexBuffer = skybox->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(skybox->GetIndexCount(), 0, 0);

	context->OMSetDepthStencilState(NULL, 0);
}

void Game::DrawSceneBlend(XMFLOAT4 clip)
{
}

void Game::DrawParticles(float totalTime, XMFLOAT4 clip)
{
	//rendering particle
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);
	context->OMSetDepthStencilState(particleDepth, 0);

	auto view = camera->GetViewMatrix();

	particlePS->SetSamplerState("sampleOptions", samplerState);

	for (int i = 0; i < emitterList.size(); i++)
	{
		emitterList[i]->Draw(context, view, camera->GetProjectionMatrix(), totalTime);
	}

	context->OMSetDepthStencilState(0, 0);
	context->OMSetBlendState(0, blend, 0xffffffff);
}

void Game::DrawWaterReflection()
{

}

void Game::RenderShadowMap()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//set depth stencil view to render everything to the shadow depth buffer
//context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(shadowDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(0, nullptr, shadowDepthStencil);

	context->RSSetViewports(1, &shadowViewport);
	context->RSSetState(shadowRasterizerState);

	/*XMFLOAT3 directionLightPosition;
	XMFLOAT3 center(0.0f, 0.0f, 0.0f);*/
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);

	//taking the center of the camera and backing up from the direction of the light
	//this is the position of the light
	//XMStoreFloat3(&directionLightPosition, XMLoadFloat3(&center) - XMLoadFloat3(&directionalLight.direction) * 10000.f);
	//creating the camera look to matrix
	auto tempLightView = XMMatrixLookAtLH(XMLoadFloat3(&lights[0].position),
		XMLoadFloat3(&ship->GetPosition()), XMLoadFloat3(&up));

	//storing the light view matrix
	XMFLOAT4X4 lightView;
	XMStoreFloat4x4(&lightView, XMMatrixTranspose(tempLightView));

	//calculating projection matrix
	XMFLOAT4X4 lightProjection;
	XMMATRIX tempLightProjection = XMMatrixOrthographicLH(100.f, 100.f,
		0.1f, 1000.0f);
	XMStoreFloat4x4(&lightProjection, XMMatrixTranspose(tempLightProjection));

	shadowVertexShader->SetShader();
	context->PSSetShader(nullptr, nullptr, 0);

	for (size_t i = 0; i < entities.size(); i++)
	{
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		shadowVertexShader->SetMatrix4x4("view", lightView);
		shadowVertexShader->SetMatrix4x4("projection", lightProjection);
		shadowVertexShader->SetMatrix4x4("worldMatrix", entities[i]->GetModelMatrix());
		shadowVertexShader->CopyAllBufferData();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}
}

void Game::DrawFullScreenQuad(ID3D11ShaderResourceView* texSRV)
{
	// First, turn off our buffers, as we'll be generating the vertex
	// data on the fly in a special vertex shader using the index of each vert
	context->IASetVertexBuffers(0, 0, 0, 0, 0);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Set up the fullscreen quad shaders
	fullScreenTriangleVS->SetShader();
	fullScreenTriangleVS->CopyAllBufferData();

	fullScreenTrianglePS->SetShaderResourceView("Pixels", texSRV);
	fullScreenTrianglePS->SetSamplerState("Sampler", samplerState);
	fullScreenTrianglePS->SetShader();
	fullScreenTrianglePS->CopyAllBufferData();

	// Draw
	context->Draw(3, 0);
}

void Game::CreateExplosion(XMFLOAT3 pos)
{
	std::shared_ptr<Emitter> explosion = std::make_shared<Emitter>(
		1000, //max particles
		1000, //particles per second
		0.7f, //lifetime
		0.03f, //start size
		1.0f, //end size
		XMFLOAT4(1, 0.3f, 0.3f, 1.0f), //start color
		XMFLOAT4(1, 0.1f, 0.1f, 0.3f), //end color
		XMFLOAT3(0, 0, 0.f), //start vel
		XMFLOAT3(5.0f, 5.0f, 5.0f), //velocity deviation range
		pos, //start position
		XMFLOAT3(0.0f, 0.0f, 0.0f), //position deviation range
		XMFLOAT4(-2, 2, -2, 2), //rotation around z axis
		XMFLOAT3(0.f, 0.f, 0.f), //acceleration
		device, particleVS, particlePS, particleTexture);

	explosion->SetTemporary(2.f);
	emitterList.emplace_back(explosion);
}

void Game::CreateSmoke(XMFLOAT3 shipPos)
{
	std::shared_ptr<Emitter> smoke = std::make_shared<Emitter>(
		300, //max particles
		50, //particles per second
		0.7f, //lifetime
		0.03f, //start size
		1.0f, //end size
		XMFLOAT4(1, 1.f, 1.f, 1.0f), //start color
		XMFLOAT4(1.f, 1.f, 0.5f, 1.f), //end color
		XMFLOAT3(-1, 1, 0.f), //start vel
		XMFLOAT3(0.2f, 0.2f, 0.2f), //velocity deviation range
		shipPos, //start position
		XMFLOAT3(0.2f, 0.2f, 0.2f), //position deviation range
		XMFLOAT4(-2, 2, -2, 2), //rotation around z axis
		XMFLOAT3(0.f, 0.f, 0.f), //acceleration
		device, particleVS, particlePS, particleTexture);

	emitterList.emplace_back(smoke);

}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//updating the camera projection matrix
	camera->CreateProjectionMatrix((float)width / height);

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	//updating the camera
	camera->Update(deltaTime);

	// add obstacles to screen
	frameCounter += deltaTime;

	if (frameCounter > 3)
	{
		XMFLOAT3 position = {
			(float)(rand() % 31 - 15),
			(float)(rand() % 11 - 5), // edit this to change y-range
			ship->GetPosition().z + 30.0f
		};

		std::shared_ptr<Obstacle> newObstacle = std::make_shared<Obstacle>(obstacleMesh, obstacleMat);


		// set position
		newObstacle->SetPosition(position);
		newObstacle->SetScale({ 3, 3, 3 });
		newObstacle->UseRigidBody();

		obstacles.emplace_back(newObstacle);
		entities.emplace_back(newObstacle);
		frameCounter = 0.0f;
	}

	//Generate building clusters
	buildingCounter += deltaTime;

	if (buildingCounter >= buildingTimer)
	{
		//reset timer and counter
		buildingTimer = rand() % 30 + 45;
		buildingCounter = 0;

		//Select building rotation
		int rot = rand() % 4;
		float xPos;
		float zPos;

		//get center of the building generation
		XMFLOAT3 center = XMFLOAT3(rand() % 120 - 60.0f, -80.0f, ship->GetPosition().z + 400.0f);

		//Create the buildings
		std::shared_ptr<Entity> building1 = std::make_shared<Entity>(buildingMesh, buildingMat);
		zPos = 40.5f;
		xPos = -4.5f;
		building1->SetPosition(XMFLOAT3((xPos * cos(rot * DirectX::XM_PIDIV2) - zPos * sin(rot * DirectX::XM_PIDIV2)) + 
			center.x, (rand() % 40 - 40) + center.y, zPos * cos(rot * DirectX::XM_PIDIV2) + xPos * sin(rot * DirectX::XM_PIDIV2) + center.z));
		building1->SetTag("Building");
		building1->UseRigidBody();
		entities.emplace_back(building1);

		std::shared_ptr<Entity> building2 = std::make_shared<Entity>(buildingMesh, buildingMat);
		zPos = 39;
		xPos = 36;
		building2->SetPosition(XMFLOAT3((xPos * cos(rot * DirectX::XM_PIDIV2) - zPos * sin(rot * DirectX::XM_PIDIV2)) + 
			center.x, (rand() % 40 - 40) + center.y, zPos * cos(rot * DirectX::XM_PIDIV2) + xPos * sin(rot * DirectX::XM_PIDIV2) + center.z));
		building2->SetTag("Building");
		building2->UseRigidBody();
		entities.emplace_back(building2);

		std::shared_ptr<Entity> building3 = std::make_shared<Entity>(buildingMesh, buildingMat);
		zPos = -19.5f;
		xPos = 72;
		building3->SetPosition(XMFLOAT3((xPos * cos(rot * DirectX::XM_PIDIV2) - zPos * sin(rot * DirectX::XM_PIDIV2)) + 
			center.x, (rand() % 40 - 40) + center.y, zPos * cos(rot * DirectX::XM_PIDIV2) + xPos * sin(rot * DirectX::XM_PIDIV2) + center.z));
		building3->SetTag("Building");
		building3->UseRigidBody();
		entities.emplace_back(building3);

		std::shared_ptr<Entity> building4 = std::make_shared<Entity>(buildingMesh, buildingMat);
		zPos = -42;
		xPos = -45;
		building4->SetPosition(XMFLOAT3((xPos * cos(rot * DirectX::XM_PIDIV2) - zPos * sin(rot * DirectX::XM_PIDIV2)) + 
			center.x, (rand() % 40 - 40) + center.y, zPos * cos(rot * DirectX::XM_PIDIV2) + xPos * sin(rot * DirectX::XM_PIDIV2) + center.z));
		building4->SetTag("Building");
		building4->UseRigidBody();
		entities.emplace_back(building4);

		std::shared_ptr<Entity> building5 = std::make_shared<Entity>(buildingMesh, buildingMat);
		zPos = 0;
		xPos = -70.5;
		building5->SetPosition(XMFLOAT3((xPos * cos(rot * DirectX::XM_PIDIV2) - zPos * sin(rot * DirectX::XM_PIDIV2)) + 
			center.x, (rand() % 40 - 40) + center.y, zPos * cos(rot * DirectX::XM_PIDIV2) + xPos * sin(rot * DirectX::XM_PIDIV2) + center.z));
		building5->SetTag("Building");
		building5->UseRigidBody();
		entities.emplace_back(building5);

		std::shared_ptr<Entity> boulder = std::make_shared<Entity>(boulderMesh, boulderMat);
		XMFLOAT4 rotationFloat;
		XMVECTOR rotVec = XMQuaternionRotationRollPitchYaw(3 * XM_PIDIV2, (rot+1) * XM_PIDIV2, 0);
		XMStoreFloat4(&rotationFloat, rotVec);
		boulder->SetRotation(rotationFloat);
		boulder->SetPosition(XMFLOAT3(34 * cos(rot * DirectX::XM_PIDIV2) +
			center.x, center.y, -34 * sin(rot * DirectX::XM_PIDIV2) + center.z));
		boulder->SetScale(XMFLOAT3(11, 11, 11));
		entities.emplace_back(boulder);
	}

	// handle bullet creation
 	if (GetAsyncKeyState(VK_SPACE) & 0x8000 && fired == false)
	{
		fired = true;
		if (bulletCounter<=MAX_BULLETS)
		{
			bulletCounter++;
			std::shared_ptr<Bullet> newBullet = std::make_shared<Bullet>(bulletMesh, material);
			newBullet->UseRigidBody();
			XMFLOAT3 bulletPos = ship->GetPosition();
			bulletPos.y += 0.5f;
			newBullet->SetScale(XMFLOAT3(0.3f, 0.3f, 0.3f));
			newBullet->SetPosition(bulletPos);

			// set bullet rotation to ship rotation - to match forwards ====
			// get current & original ship rotations and match them
			XMFLOAT4 storedShipRot = ship->GetRotation();
			XMFLOAT4 storedShipOrig = ship->GetOriginalRotation();
			XMVECTOR shipRotation = XMLoadFloat4(&storedShipRot);
			XMVECTOR originalRotation = XMLoadFloat4(&storedShipOrig);
			// create var to store new rotation in & store it
			XMFLOAT4 bulletRotation;
			XMStoreFloat4(
				&bulletRotation,
				XMQuaternionMultiply(XMQuaternionInverse(originalRotation), shipRotation)
			);
			newBullet->SetRotation(bulletRotation);
			entities.emplace_back(newBullet);
		}
	}

	if (GetAsyncKeyState(VK_SPACE) == 0 && fired == true)
	{
		fired = false;
	}

	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetAliveState())
		{
			entities[i]->Update(deltaTime);
			if (ship->GetHealth() < 4)
			{
				//CreateSmoke(ship->GetPosition());
			}
		}
	}

	auto shipPos = ship->GetPosition();
	auto shipForward = ship->GetForward();
	XMFLOAT3 em1Pos;
	XMStoreFloat3(&em1Pos, XMLoadFloat3(&shipPos) + XMLoadFloat3(&shipForward));
	em1Pos.x -= 1.2f * ship->GetScale().x;
	shipGas->SetAcceleration(shipForward);
	shipGas->SetPosition(em1Pos);

	shipPos = ship->GetPosition();
	XMFLOAT3 em2Pos;
	XMStoreFloat3(&em2Pos, XMLoadFloat3(&shipPos) + XMLoadFloat3(&shipForward));
	em2Pos.x += 1.2f * ship->GetScale().x;
	shipGas2->SetAcceleration(shipForward);
	shipGas2->SetPosition(em2Pos);

	for (int i = 0; i < emitterList.size(); i++)
	{
		emitterList[i]->UpdateParticles(deltaTime, totalTime);
		if (emitterList[i]->IsDead())
		{
			emitterList[i] = nullptr;
		}
	}

	water->Update(deltaTime, ship->GetPosition());
	XMFLOAT3 terrainPos = shipPos;

	terrainPos.y = -20.0f;
	terrainPos.x = -5;
	terrain->SetPosition(terrainPos);

	//checking for collision
	for (int i = 0; i < entities.size(); i++)
	{
		for (int j = 0; j < entities.size(); j++)
		{
			entities[i]->IsColliding(entities[j]);
		}
	}


	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetAliveState() == false)
		{
			if (entities[i]->GetTag() == "bullet") 
			{
				bulletCounter--;
			}

			if (entities[i]->GetTag() == "Player")
			{
				//std::thread t1(&Game::RestartGame,this);
				//t1.detach();
				RestartGame();
				//entities[i] = nullptr;
				break;
			}

			if (entities[i]->GetTag() == "Obstacle")
			{
				CreateExplosion(entities[i]->GetPosition());
			}

			entities[i] = nullptr;
		}
	}

	//delete entities behind player
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i] == NULL)
		{
			break;
		}

		if (entities[i]->GetPosition().z < ship->GetPosition().z - 100.0f)
		{
			entities[i]->Die();
		}
	}

	XMFLOAT4 shipFront = XMFLOAT4(ship->GetPosition().x, ship->GetPosition().y, ship->GetPosition().z + 20, 1.0f);

	XMVECTOR clipSpace = XMVector4Transform(XMVector4Transform(XMLoadFloat4(&shipFront), XMMatrixTranspose(XMLoadFloat4x4(&camera->GetViewMatrix()))), XMMatrixTranspose(XMLoadFloat4x4(&camera->GetProjectionMatrix())));

	XMFLOAT4 ssPos;

	XMStoreFloat4(&ssPos, clipSpace);

	uvCoord = XMFLOAT2((ssPos.x / ssPos.w) * 0.5f + 0.5f, (ssPos.y / ssPos.w) * -0.5f + 0.5f);

	entities.erase(std::remove(entities.begin(), entities.end(), nullptr), entities.end());
	emitterList.erase(std::remove(emitterList.begin(), emitterList.end(), nullptr), emitterList.end());

	lights[1].position.x = (float)sin(deltaTime) * 10;
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(waterReflectionRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);

	//rendering shadow
	RenderShadowMap();

	//rendering the scene without water
	context->OMSetRenderTargets(1, &waterReflectionRTV, depthStencilView);

	context->RSSetState(nullptr);
	context->RSSetViewports(1, &viewport);

	XMFLOAT4 clip = XMFLOAT4(0, 1.0f, 0, 10.f);

	reflect = true;
	auto cameraPos = camera->GetPosition();
	float distance = 2 * (cameraPos.y - (-1.0f));
	cameraPos.y -= distance;
	camera->SetPosition(cameraPos);
	camera->InvertPitch();

	DrawSceneOpaque(clip);
	DrawSky(clip);
	DrawParticles(totalTime,clip);

	reflect = false;
	camera->InvertPitch();
	cameraPos.y += distance;
	camera->SetPosition(cameraPos);
	//context->OMSetRenderTargets(1, &backBufferRTV, 0);
	//rendering full screen quad for reflection
	//DrawFullScreenQuad(waterReflectionSRV);

	//stride of each vertex
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(ppRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->RSSetState(nullptr);
	context->RSSetViewports(1, &viewport);

	// --- Post Processing! ---------------------
	// Change the render target
	context->OMSetRenderTargets(1, &ppRTV, depthStencilView);
	
	clip = XMFLOAT4(0, 0, 0, 0);
	DrawSceneOpaque(clip);

	//drawing the water
	waterPS->SetShaderResourceView("reflectionTexture", waterReflectionSRV);
	waterPS->SetShaderResourceView("foam", foam);
	water->Draw(lights[0], skybox->GetSkyboxTexture(), camera, context,deltaTime,totalTime,waterSampler);

	DrawSky(clip);

	DrawParticles(totalTime,clip);

	/*terrain->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix(),
		context, lights[0]);*/

	// Reset states for next frame
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	// --- Post processing -----------------------
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	// Set up post process shaders
	ppVS->SetShader();

	ppPS->SetShaderResourceView("Pixels", ppSRV);
	ppPS->SetSamplerState("Sampler", ppSampler);
	ppPS->SetShader();

	ppPS->SetFloat2("uvCoord", uvCoord);
	ppPS->SetFloat("sampleStrength", 1.5f);
	ppPS->SetFloat("sampleDistance", 0.5f);
	ppPS->CopyAllBufferData();

	// Turn OFF vertex and index buffers
	ID3D11Buffer* nothing = 0;
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);

	// Draw exactly 3 vertices for our "full screen triangle"
	context->Draw(3, 0);

	ID3D11ShaderResourceView* nullSRV[16] = {};
	context->PSSetShaderResources(0, 16, nullSRV);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain effect,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1,&backBufferRTV, depthStencilView);

	
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	//sending the mouse data to the camera

	//how much the mouse has moved since the previous call to this function

	if (buttonState & 0x0001)
	{
		int deltaX = x - prevMousePos.x;
		int deltaY = y - prevMousePos.y;

		//changing the yaw and pitch of the camera
		camera->ChangeYawAndPitch((float)deltaX, (float)deltaY);
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion