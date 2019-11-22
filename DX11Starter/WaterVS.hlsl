
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	float4 clipDistance;
	/*float amplitude;
	float steepness;
	float wavelength;
	float speed;*/
	float dt;
	float4 waveA;
	float4 waveB;
	float4 waveC;
	float4 waveD;
	float2 direction;
	matrix projection;
	matrix lightView;
	matrix lightProj;
};


static const float PI = 3.14159f;

// Struct representing a single vertex worth of data
struct VertexShaderInput
{

	float3 position		: POSITION;     // XYZ position
	//float4 color		: COLOR;        // RGBA color
	float3 normal		: NORMAL;		//Normal of the vertex
	float3 tangent		: TANGENT;      //tangent of the vertex
	float2 uv			: TEXCOORD;		//Texture coordinates
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel
{

	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
	noperspective float2 screenUV		: VPOS;
};

//function to calculate gerstner waves given a wave
float3 GerstnerWave(float4 wave, float3 pos, inout float3 tangent, inout float3 binormal, float dt)
{
	float steepness = wave.z;
	float wavelength = wave.w;

	float k = 2 * PI / wavelength;
	//modifying the y value and creating gerstner waves
	float c = sqrt(9.8f / k); //phase speed
	float2 d = normalize(wave.xy);
	float a = steepness / k;
	float f = k * (dot(d, pos.xz) - c / 2 * dt);

	tangent += float3(
		 - d.x * d.x * (steepness * sin(f)),
		d.x * (steepness * cos(f)),
		-d.x * d.y * (steepness * sin(f)
			)
		);

	binormal += float3(
		-d.x * d.y * (steepness * sin(f)),
		d.y * (steepness * cos(f)),
		 - d.y * d.y * (steepness * sin(f))
		);

	return float3(
		d.x * (a * cos(f)),
		a * sin(f),
		d.y * (a * cos(f))
	);
}
Texture2D heightMap: register(t4);
SamplerState sampleOptions: register(s0);
// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	// First we multiply them together to get a single matrix which represents
	// all of those transformations (world to view to projection space)
	matrix worldViewProj = mul(mul(world, view), projection);

	float height = heightMap.Sample(sampleOptions,input.uv);

	float3 pos = input.position;
	float3 tangent = float3(1.0f, 0, 0);
	float3 binormal = float3(0.0f, 0, 1.0f);
	pos += GerstnerWave(waveA,input.position,tangent,binormal,dt);
	pos += GerstnerWave(waveB, input.position, tangent, binormal, dt);
	pos += GerstnerWave(waveC, input.position, tangent, binormal, dt);
	pos += GerstnerWave(waveD, input.position, tangent, binormal, dt);
	input.position = pos;
	input.normal = normalize(cross(binormal,tangent));

	// The result is essentially the position (XY) of the vertex on our 2D 
	// screen and the distance (Z) from the camera (the "depth" of the pixel)
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	//applying the normal by removing the translation from it
	output.normal = mul(input.normal, (float3x3)world);

	//sending the world position of the vertex to the fragment shader
	output.worldPosition = mul(float4(input.position, 1.0f), world).xyz;

	//sending the world coordinates of the tangent to the pixel shader
	output.tangent = mul(tangent, (float3x3)world);

	//sending the UV coordinates
	output.uv = input.uv;

	matrix lightWorldViewProj = mul(mul(world, lightView), lightProj);

	output.screenUV = output.position.xy / output.position.w;
	output.screenUV.x = output.screenUV.x * 0.5 + 0.5;
	output.screenUV.y = -output.screenUV.y * 0.5 + 0.5;

	//sending the the shadow position
	output.lightPos = mul(float4(input.position, 1.0f), lightWorldViewProj);

	// Whatever we return will make its way through the pipeline to the pixel shader
	return output;
}