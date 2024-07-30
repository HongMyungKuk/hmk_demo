#include "Common.hlsli"

Texture2D albedoTexture : register(t10);
Texture2D metalnessTexture : register(t11);
Texture2D roughnessTexture : register(t12);

#define PI 3.141592

static const float3 Fdielectric = 0.04;

float NdfGGX(float hdotn, float roughness)
{
    float alphaSrq = roughness * roughness;
    float denom = ((hdotn * hdotn) * (alphaSrq - 1) + 1);
    
    return alphaSrq / (PI * denom * denom);
}

float SchlickG1(float k, float c)
{
    return c / (c * (1 - k) + k);
}

float SchlickGGX(float ndotl, float ndotv, float roughness)
{
    float alpha = (roughness + 1) * (roughness + 1);
    float k = alpha * alpha / 8.0;
    
    return SchlickG1(k, ndotl) * SchlickG1(k, ndotv);
}

float3 SchlickFresnel(float3 F0, float vdoth)
{
    return F0 + (1 - F0) * pow(2.0, (-5.55473 * (vdoth) - 6.98316) * vdoth);
}

float3 CalcIrradiance(Light L, float3 posWorld, float3 toEye, float3 normalWorld, Texture2D shadowMap)
{
    float3 lightVec = L.position - posWorld;
    float distance = length(lightVec);
    
    lightVec /= distance;
    
    float att = CalcAttenuation(L.fallOffStart, L.fallOffEnd, distance);
    
    float spotPower = 1.0;
    spotPower = (L.type & SPOT_LIGHT) ? pow(max(0.0, dot(normalize(L.direction), -lightVec)), L.spotPower) : 1.0;
    
    
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
        
        float depthSrc = shadowMap.Sample(shadowPointDynamicSS, float2(texCoord.x, texCoord.y)).r;
        
        // set border value.
        if (texCoord.x < 0.0f || texCoord.x > 1.0f || texCoord.y < 0.0f || texCoord.y > 1.0f)
            depthSrc = 100.0f;
        
        float2 ab = texCoord;

        if (depthSrc + 0.001 < depthDest)
        {
            shadowFactor = 0.0;
        }
    }
        
    return L.irRadiance * shadowFactor * att * spotPower;
}

float4 main(PSInput input) : SV_TARGET
{
    float3 color = 0.0;

    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = input.normalWorld;
    
    float3 albedo = useAlbedoMap ? albedoTexture.Sample(linearWrapSS, input.texCoord).xyz : albedoFactor;
    float metalness = useMetalnessMap ? metalnessTexture.Sample(linearWrapSS, input.texCoord).r : metalnessFactor;
    float roughness = useRoughnessMap ? roughnessTexture.Sample(linearWrapSS, input.texCoord).r : roughnessFactor;
    float3 emission = useEmissiveMap ? float3(1.0, 1.0, 1.0) : emissionFactor;
    
    // Image Based Lighting.
    
    
    
    
    
    // Shading Model
    float3 directLighting = 0.0;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (light[i].type)
        {
            Light L = light[i];
            
            float3 lightVec = 0.0;
            if (L.type & DIRECTIONAL_LIGHT)
                lightVec = normalize(-L.direction);
            if (L.type & POINT_LIGHT || L.type & SPOT_LIGHT)
                lightVec = normalize(L.position - input.posWorld);
            
            float3 halfWay = normalize(toEye + lightVec);
            float hdotn = max(0.0, dot(halfWay, normalWorld));
            float ndotl = max(0.0, dot(normalWorld, lightVec));
            float vdoth = max(0.0, dot(toEye, halfWay));
            float ndotv = max(0.0, dot(normalWorld, toEye));
            
            // metal 값이 올라가면 albedo에 가까워짐
            float3 F0 = lerp(Fdielectric, albedo, metalness);
            float3 F = SchlickFresnel(F0, vdoth);
            float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);
            
            float3 diffuseBRDF = kd * albedo;
            
            float D = NdfGGX(hdotn, roughness);
            float G = SchlickGGX(ndotl, ndotv, roughness);
            
            float3 specularBRDF = (F * D * G) / max(1e-7, 4.0 * ndotl * ndotv);
            
            float3 irRadiance = 0.0;
            if (i == 0)
                irRadiance = CalcIrradiance(L, input.posWorld, toEye, normalWorld, shadowMap0);
            if (i == 1)
                irRadiance = CalcIrradiance(L, input.posWorld, toEye, normalWorld, shadowMap1);
            if (i == 2)
                irRadiance = CalcIrradiance(L, input.posWorld, toEye, normalWorld, shadowMap2);
            
            directLighting += (diffuseBRDF + specularBRDF) * irRadiance * ndotl;
        }
    }
    
    return float4(directLighting + emission, 1.0);
}