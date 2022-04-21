// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float2 pos : POSITION;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD;
};

cbuffer data
{
    // THIS IS NOT CONTIGOUS IN MEMORY, EVERY ROW IS SEPARATED BY 4 BYTES CAUSE EVERY ROW IS A 
    // SEPARATE VECTOR OF 3 FLOAT3S SO IT SEPARATES THEM INTO 3 DIFFERENT FLOAT3 ROWS
    float3x3 transform;
    float2 atlasSize;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    //float3 post = mul(transform, float3(input.pos, 1.0f));
    //float4 pos = float4(post, 1.0f);
    
  //  float3 resMul = float3(
  //      input.pos.x * transform._m00 + input.pos.y * transform._m10 + 1.f * transform._m20,
		//input.pos.x * transform._m01 + input.pos.y * transform._m11 + 1.f * transform._m21,
		//input.pos.x * transform._m02 + input.pos.y * transform._m12 + 1.f * transform._m22
  //  );
  //  float4 pos = float4(resMul, 1.f);
    
    float4 pos = float4(mul(transform, float3(input.pos, 1.0f)), 1.0f);
    
    //float4 pos = float4(input.pos, 0.f, 1.0f);
    
    
	// Transform the vertex position into projected space.
    output.pos = pos;

	// Pass the color through without modification.
    output.color = input.color;
    
    //output.texcoord = input.texcoord;
    output.texcoord = float2(input.texcoord.x / atlasSize.x, input.texcoord.y / atlasSize.y);

    return output;
}
