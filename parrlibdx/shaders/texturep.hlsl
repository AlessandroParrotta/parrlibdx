Texture2D shaderTexture : register(t0); //slot 0
SamplerState SampleType;
//SamplerState SampleType : register(s0); //slot 0

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    return shaderTexture.Sample(SampleType, input.texcoord)*input.color;
}
