#include "Common.hlsli"

Texture2D albedoTexture : register(t1);

float4 main(PSInput input) : SV_TARGET
{
    float3 color = 0.0;
    
    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = input.normalWorld;
    
    //int i = 0;
    //[unroll]
    //for (i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
    //{
    //    color += ComputeDirectionalLight(light[i], toEye, normalWorld);
    //}
    //[unroll]
    //for (i = NUM_DIRECTIONAL_LIGHTS; i < NUM_DIRECTIONAL_LIGHTS + NUM_POINT_LIGHTS; i++)
    //{
    //    color += ComputePointLights(light[i], input.posWorld, toEye, normalWorld);
    //}
    //[unroll]
    //for (i = NUM_DIRECTIONAL_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIRECTIONAL_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; i++)
    //{
    //    color += ComputeSpotLight(light[i], input.posWorld, toEye, normalWorld);
    //}
    
    int i = 0;
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        if (light[i].type)
        {
            Light L = light[i];
            
            color += (L.type & DIRECTIONAL_LIGHT) ? ComputeDirectionalLight(L, toEye, normalWorld) : float3(0.0, 1.0, 0.0);
            color += (L.type & POINT_LIGHT) ? ComputePointLights(L, input.posWorld, toEye, normalWorld) : float3(0.0, 1.0, 0.0);
            color += (L.type & SPOT_LIGHT) ? ComputeSpotLight(L, input.posWorld, toEye, normalWorld) : float3(0.0, 1.0, 0.0);
        }
    }
    
    if (texFlag)
    {
        color = albedoTexture.Sample(linearWrapSS, input.texCoord).xyz;
    }
    
    return float4(color, 1.0);
}