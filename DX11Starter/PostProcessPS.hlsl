
cbuffer Data : register(b0)
{
    float2 uvCoord;
    float sampleStrength;
    float sampleDistance;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Textures and such
Texture2D Pixels : register(t0);
SamplerState Sampler : register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
    float2 direction = uvCoord - input.uv;
    float distance = length(direction);
    direction = direction / distance;
    float4 thisPixel = Pixels.Sample(Sampler, input.uv);
    float4 totalColor = thisPixel;
    
    float samples[10];
    samples[0] = -0.08;
    samples[1] = -0.05;
    samples[2] = -0.03;
    samples[3] = -0.02;
    samples[4] = -0.01;
    samples[5] = 0.01;
    samples[6] = 0.02;
    samples[7] = 0.03;
    samples[8] = 0.05;
    samples[9] = 0.08;
    
    
    for (int i = 0; i < 10; i++)
    {
        totalColor += Pixels.Sample(Sampler, input.uv + direction * samples[i] * sampleDistance);
    }
    totalColor *= 1.0 / 11.0;
    float t = distance * sampleStrength;
    t = clamp(t, 0.0, 1.0);
    
    return lerp(thisPixel, totalColor, t);
}