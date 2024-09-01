struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PixelShaderInput
{
    float4 posModel : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posModel = float4(input.posModel, 1.0);
    output.texCoord = input.texCoord;
    
    return output;
}