#include "Common.hlsli"

struct VertexShaderInput
{
    float3 posModel : POSITION;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

PixelShaderInput vsmain(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posModel = input.posModel;
    output.posProj = mul(float4(input.posModel, 1.0), world);
    output.posProj = mul(float4(output.posProj.xyz, 0.0), view);
    output.posProj = mul(output.posProj, proj);
    
    return output;
}

float4 psmain(PixelShaderInput input) : SV_TARGET
{
    return envTexture.Sample(linearWrapSS, input.posModel);
}