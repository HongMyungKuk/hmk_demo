#include "Common.hlsli"

Texture2D depthMap : register(t0);

struct PixelShaderInput
{
    float4 posModel : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{   
    return depthMap.Sample(linearWrapSS, input.texCoord);

}