#include "Common.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 color = 0.0;
    
    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = input.normalWorld;
    
    if (texFlag)
    {
        color = albedoTexture.Sample(linearWrapSS, input.texCoord).xyz;
    }
    else
    {
        color = ambient;
    }
    
    int i = 0;
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        if (light[i].type)
        {
            Light L = light[i];
            
            // �̷��� ����� 3���� ������ ��ø ������ �ʹ� �������.
            // �ٸ� ����� ����...
            color += (L.type & DIRECTIONAL_LIGHT) ? ComputeDirectionalLight(L, toEye, normalWorld) : 0.0;
            color += (L.type & POINT_LIGHT) ? ComputePointLights(L, input.posWorld, toEye, normalWorld) : 0.0;
            color += (L.type & SPOT_LIGHT) ? ComputeSpotLight(L, input.posWorld, toEye, normalWorld) : 0.0;
        }
    }
    
    return float4(color, 1.0);
}