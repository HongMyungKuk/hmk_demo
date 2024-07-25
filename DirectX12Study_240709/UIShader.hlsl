#include "Common.hlsli"

PSInput vsmain(VSInput input)
{
    PSInput output;

    output.posModel = input.posModel;
    
    output.posProj = mul(float4(input.posModel, 1.0), world);
    output.posWorld = output.posProj.xyz;
    //output.posProj = mul(float4(output.posProj.xyz, 1.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);

    output.normalWorld = input.normalModel;
    output.texCoord = input.texCoord;
    
    return output;
}
