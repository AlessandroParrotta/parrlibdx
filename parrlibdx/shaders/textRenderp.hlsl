Texture2D shaderTexture : register(t0); //slot 0
SamplerState SampleType;

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD;
};

cbuffer data
{
    float3x3 transform;
    float2 atlasSize;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    return shaderTexture.Sample(SampleType, input.texcoord) * input.color;
    //return float4(input.texcoord.x, input.texcoord.y, 0.f, 1.f);
    //return float4(1.f, 1.f, 1.f, 1.f);
}
