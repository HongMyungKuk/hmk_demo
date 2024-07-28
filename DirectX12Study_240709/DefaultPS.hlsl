#include "Common.hlsli"

float3 CalcLightColor(float3 albedo, Light L, float3 posModel, float3 posWorld, float3 toEye, float3 normalWorld)
{
    float3 color = 0.0;

    if (L.type & DIRECTIONAL_LIGHT)
        color = ComputeDirectionalLight(albedo, L, toEye, normalWorld);
    else if (L.type & POINT_LIGHT)
        color = ComputePointLights(albedo, L, posWorld, toEye, normalWorld);
    else if (L.type & SPOT_LIGHT)
        color = ComputeSpotLight(albedo, L, posWorld, toEye, normalWorld);
    else
        color = 0.5;
    
    return color;
}

float3 CalcIrradiance(float3 albedo, Light L, uint idx, float3 posModel, float3 posWorld, float3 toEye, float3 normalWorld, Texture2D shadowMap)
{
    float3 color = CalcLightColor(albedo, L, posModel, posWorld, toEye, normalWorld);
    
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
        float depthSrc = shadowMap.Sample(shadowPointSS, float2(texCoord.x, texCoord.y)).r;
        
        float2 ab = texCoord;

        if (depthSrc + 0.0015 < depthDest)
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
    
    float3 albedo = useAlbedoMap ? albedoTexture.Sample(linearWrapSS, input.texCoord).xyz : albedoFactor;
    float3 emission = useEmissiveMap ? float3(1.0, 1.0, 1.0) : emissionFactor;
    
    int i = 0;
    
    [unroll]
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        if (light[i].type)
        {
            Light L = light[i];
            
            // 이러한 방식은 3개의 조명이 중첩 됐을때 너무 밝아진다.
            // 다른 방식을 강구...
            if (i == 0)
                color += CalcIrradiance(albedo, L, i, input.posModel, input.posWorld, toEye, input.normalWorld, shadowMap0);
            if (i == 1)
                color += CalcIrradiance(albedo, L, i, input.posModel, input.posWorld, toEye, input.normalWorld, shadowMap1);
            if (i == 2)
                color += CalcIrradiance(albedo, L, i, input.posModel, input.posWorld, toEye, input.normalWorld, shadowMap2);
        }
    }
    
    //return float4(color + emission, 1.0);
    return albedoTexture.Sample(linearWrapSS, input.texCoord);
}