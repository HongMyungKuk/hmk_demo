#include "Common.hlsli"

float3 CalcLightColor(Light L, float3 posModel, float3 posWorld, float3 toEye, float3 normalWorld)
{
    float3 color = 0.0;

    if (L.type & DIRECTIONAL_LIGHT)
        color = ComputeDirectionalLight(L, toEye, normalWorld);
    else if(L.type & POINT_LIGHT)
        color = ComputePointLights(L, posWorld, toEye, normalWorld);
    else if(L.type & SPOT_LIGHT)
        color = ComputeSpotLight(L, posWorld, toEye, normalWorld);
    else
        color = 0.5;
    
    return color;
}

float3 CalcIrradiance(Light L, uint idx, float3 posModel, float3 posWorld, float3 toEye, float3 normalWorld, Texture2D shadowMap)
{ 
    float3 color = CalcLightColor(L, posModel, posWorld, toEye, normalWorld);
    
    float shadowFactor = 1.0;
    
    if (L.type & SHADOW_MAP)
    {
        float4 lightScreen = mul(float4(posWorld, 1.0), L.view);
        lightScreen = mul(lightScreen, L.proj);
        lightScreen.xyz /= lightScreen.w;
    
        float2 texCoord = float2(lightScreen.x, -lightScreen.y);
        texCoord += 1.0;
        texCoord *= 0.5;
    
        float depthDest = lightScreen.z;
        float depthSrc = shadowMap.Sample(shadowPointDynamicSS, texCoord).r;
        //float a = shadowMap.Sample(shadowPointDynamicSS, texCoord).g;
        //float b = shadowMap.Sample(shadowPointDynamicSS, texCoord).b;
        //float c = shadowMap.Sample(shadowPointDynamicSS, texCoord).a;
    
        if (depthSrc + 0.0001 < depthDest)
        {
            shadowFactor = 0.0;
        }
    }
        
    return color * shadowFactor;
}

float4 main(PSInput input) : SV_TARGET
{
    float3 color = 0.0;

    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = input.normalWorld;
    
    //if (texFlag)
    //{
    //    color = albedoTexture.Sample(linearWrapSS, input.texCoord).xyz;
    //}
    //else
    //{
    //    color = ambient;
    //}
    
    int i = 0;
    
    [unroll]
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        if (light[i].type)
        {
            Light L = light[i];
            
            // 이러한 방식은 3개의 조명이 중첩 됐을때 너무 밝아진다.
            // 다른 방식을 강구...
            color += CalcIrradiance(L, i, input.posModel, input.posWorld, toEye, input.normalWorld, shadowMap[i]);
        }
    }
    
    return float4(color, 1.0);
}