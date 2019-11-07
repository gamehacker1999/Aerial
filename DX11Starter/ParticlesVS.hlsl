cbuffer externalData: register(b0)
{
	matrix view;
	matrix projection;
};

struct ParticleVertex
{
	float3 position: POSITION;
	float2 uv: TEXCOORD;
	float4 color: COLOR;
};

struct VertexToPixel
{
	float4 position: SV_POSITION;
	float2 uv : TEXCOORD;
	float4 color: COLOR;
};

VertexToPixel main(ParticleVertex input)
{
	VertexToPixel output;

	matrix viewProj = mul(view, projection);

	output.position = mul(float4(input.position, 1.0f), viewProj);
	output.uv = input.uv;
	output.color = input.color;

	return output;
}