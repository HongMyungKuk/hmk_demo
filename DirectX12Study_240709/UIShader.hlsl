#include "Common.hlsli"

Texture2D albedoTexture[7] : register(t3);

PSInput vsmain(VSInput input)
{
    PSInput output;

    output.posProj = mul(float4(input.posModel, 1.0), world);
    //output.posProj = mul(float4(output.posProj.xyz, 1.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);

    output.normalWorld = input.normalModel;
    output.texCoord = input.texCoord;
    
    return output;
}
