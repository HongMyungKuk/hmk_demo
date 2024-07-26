#include "Common.hlsli"

struct PixelShaderInput
{
    float4 posModel : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float3 TextureToView(float2 texCoord)
{
    float4 p = 0.0;
    
    p.x = texCoord.x * 2.0 - 1.0;
    p.y = -texCoord.y * 2.0 + 1.0;
    p.z = shadowMap0.Sample(pointClampSS, texCoord).r;
    p.w = 1.0;
    
    p = mul(p, projInv);
    p /= p.w;
    
    return p.xyz;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float z = TextureToView(input.texCoord).z;
    
    return float4(z, z, z, 1.0) * 0.05f;
}