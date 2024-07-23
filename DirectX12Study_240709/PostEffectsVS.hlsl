#include "Common.hlsli"

struct PixelShaderInput
{
    float4 posModel : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PixelShaderInput main(VSInput input)
{
    PixelShaderInput output;
    
    output.posModel = float4(input.posModel, 1.0);
    output.texCoord = input.texCoord;
    
    return output;
}