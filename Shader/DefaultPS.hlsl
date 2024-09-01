#include "Common.hlsli"

Texture2D albedoTexture : register(t4);
Texture2D metalnessTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D normalTexture : register(t7);
Texture2D aoTexture : register(t9);
Texture2D emissionTexture : register(t10);


#define PI 3.141592

static const float3 Fdielectric = 0.04;

float3 SchlickFresnel(float3 F0, float vdoth)
{
    return F0 + (1 - F0) * pow(2.0, (-5.55473 * (vdoth) - 6.98316) * vdoth);
}

float3 DiffuseIBL(float3 albedo, float3 toEye, float3 normalWorld, float metalness)
{
    float3 F0 = lerp(Fdielectric, albedo, metalness);
    float3 F = SchlickFresnel(F0, max(0.0, dot(toEye, normalWorld)));
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);
    float3 irradiance = diffuseTexture.Sample(linearWrapSS, normalWorld).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 toEye, float3 normalWorld, float metalness, float roughness)
{
    float2 BRDF = brdfTexture.SampleLevel(pointClampSS, float2(max(0.0, dot(toEye, normalWorld)), 1 - roughness), 0.0).rg;
    float3 irradiance = specularTexture.SampleLevel(linearWrapSS, -reflect(toEye, normalWorld), 2 * roughness + 3).rgb;
    float3 F0 = lerp(Fdielectric, albedo, metalness);

    return (F0 * BRDF.x + BRDF.y) * irradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 toEye, float3 normalWorld, float roughness, float metalness, float ao)
{
    float3 diffuseIBL = DiffuseIBL(albedo, toEye, normalWorld, metalness);
    float3 specularIBL = SpecularIBL(albedo, toEye, normalWorld, metalness, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
}

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

float3 CalcIrradiance(Light L, float3 posWorld, float3 toEye, float3 normalWorld, Texture2D shadowMap)
{
    
    float3 lightVec = 0.0;
    
    if (L.type & DIRECTIONAL_LIGHT)
        lightVec = normalize(-L.direction);
    
    float att = 1.0;
    float spotPower = 1.0;
    if (L.type & POINT_LIGHT || L.type & SPOT_LIGHT)
    {
        lightVec = normalize(L.position - posWorld);
    
        float distance = length(lightVec);
    
        lightVec /= distance;
    
        att = CalcAttenuation(L.fallOffStart, L.fallOffEnd, distance);
    
        spotPower = (L.type & SPOT_LIGHT) ? pow(max(0.0, dot(normalize(L.direction), -lightVec)), L.spotPower) : 1.0;
    }
    
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
        
        // 물체를 비추는 방향의 반대쪽에 그림자가 생기는것을 방지하기 위해 예외처리
        if (depthDest > 1.0)
        {
            depthSrc = 100.0f;
        }
        
        // set border value.
        if (texCoord.x < 0.0f || texCoord.x > 1.0f || texCoord.y < 0.0f || texCoord.y > 1.0f)
        {
            depthSrc = 100.0f;
        }
        
        float2 ab = texCoord;

        if (depthSrc + 0.001 < depthDest)
        {
            shadowFactor = 0.0;
        }
    }
        
    return L.irRadiance * shadowFactor * att * spotPower;
}

float3 GetNoramlWorld(PSInput input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (mapFlags & 0x10)
    {
        float3 normal = normalTexture.Sample(linearClampSS, input.texCoord).xyz;
        normal = 2.0 * normal - 1.0;
        
        // This case => OpenGL file.
        if(mapFlags & 0x40)
        {
            normal.y *= -1.0;
        }
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        float3x3 TBN = float3x3(T, B, N);
        
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = GetNoramlWorld(input);
    
    float3 albedo = (mapFlags & 0x01) ? albedoTexture.Sample(linearWrapSS, input.texCoord).xyz : albedoFactor;
    float metalness = (mapFlags & 0x02) ? metalnessTexture.Sample(linearWrapSS, input.texCoord).r : metalnessFactor;
    float roughness = (mapFlags & 0x04) ? roughnessTexture.Sample(linearWrapSS, input.texCoord).r : roughnessFactor;
    float3 emission = (mapFlags & 0x08) ? emissionTexture.Sample(linearWrapSS, input.texCoord).rgb : emissionFactor;
    float ao = (mapFlags & 0x40) ? aoTexture.Sample(linearWrapSS, input.texCoord).r : 1.0;
    
    // Image Based Lighting.
    float3 ambientLighting = 0.0;
    
    ambientLighting = AmbientLightingByIBL(albedo, toEye, normalWorld, roughness, metalness, ao) * envStrength;
    
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
    
    output.pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
    
    return output;
}