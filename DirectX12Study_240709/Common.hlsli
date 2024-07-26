#define MAX_LIGHTS 3
#define NUM_DIRECTIONAL_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1
#define DIRECTIONAL_LIGHT 0x01
#define POINT_LIGHT       0x02
#define SPOT_LIGHT        0x04
#define LIGHT_OFF         0x00
#define SHADOW_MAP        0x10

SamplerState linearWrapSS : register(s0);
SamplerState linearClampSS : register(s1);
SamplerState pointWrapSS : register(s2);
SamplerState pointClampSS : register(s3);
SamplerState shadowPointSS : register(s4);
SamplerState shadowPointDynamicSS : register(s5);

TextureCube envTexture : register(t0);
Texture2D albedoTexture : register(t1);
Texture2D shadowMap0 : register(t2);
Texture2D shadowMap1 : register(t3);
Texture2D shadowMap2 : register(t4);

// #define SKINNED 1

#ifdef SKINNED
cbuffer SkinnedMeshConsts : register(b3)
{
    Matrix bonesTransform[67];
}
#endif

struct Light
{
    float3 direction;
    float shininess;
    float3 position;
    float spotPower;
    float3 irRadiance;
    float fallOffStart;
    float fallOffEnd;
    uint type;
    float2 dummy;
    // shadow matrix
    Matrix view;
    Matrix proj;
};

cbuffer GloabalConsts : register(b0)
{
    Matrix view;
    Matrix viewInv;
    Matrix proj;
    Matrix projInv;
    Matrix viewProjInv;
    float3 eyeWorld;
    float dt;

    Light light[MAX_LIGHTS];
}

cbuffer MeshConsts : register(b1)
{
    Matrix world;
    Matrix worldIT;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor;
    uint useAlbedoMap;
    float3 diffuse;
    uint dummy1;
    float3 specular;
    uint dummy2;
    float3 emissionFactor;
    uint useEmissiveMap;
};

struct VSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texCoord : TEXCOORD;
    
#ifdef SKINNED
    float4 boneWeights0 : BLENDWEIGHT0;
    float4 boneWeights1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
#endif
};

struct PSInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION0;
    float3 posWorld : POSITION1;
    float3 normalWorld : NORMAL;
    float2 texCoord : TEXCOORD;
};

float CalcAttenuation(float fallOffStart, float fallOffEnd, float d)
{
    return saturate((fallOffEnd - d) / (fallOffEnd - fallOffStart));
}

// Compute Light.
float3 BlinnPhong(float3 albedo, Light L, float3 lightVec, float3 lightStrength, float3 toEye, float3 normalWorld)
{
    float3 halfWay = normalize(toEye + lightVec);
    float ndoth = dot(halfWay, normalWorld);
    
    float3 _specular = specular * pow(max(0.0, ndoth), L.shininess);
    
    return albedo + (diffuse + _specular) * lightStrength;
}

float3 ComputeDirectionalLight(float3 albedo, Light L, float3 toEye, float3 normalWorld)
{
    float3 lightVec = normalize(-L.direction);
    
    float ndotl = dot(lightVec, normalWorld);
    float3 lightStrength = max(0.0, ndotl) * L.irRadiance;
    
    return BlinnPhong(albedo, L, lightVec, lightStrength, toEye, normalWorld);
}

float3 ComputePointLights(float3 albedo, Light L, float3 posWorld, float3 toEye, float3 normalWorld)
{
    float3 lightVec = L.position - posWorld;
    
    float d = length(lightVec);
    
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;
        
        float ndotl = max(0.0, dot(lightVec, normalWorld));
        float3 lightStrength = ndotl * L.irRadiance;
        
        float att = CalcAttenuation(L.fallOffStart, L.fallOffEnd, d);
        
        lightStrength *= att;
        
        return BlinnPhong(albedo, L, lightVec, lightStrength, toEye, normalWorld);
    }
}

float3 ComputeSpotLight(float3 albedo, Light L, float3 posWorld, float3 toEye, float3 normalWorld)
{
    float3 lightVec = L.position - posWorld;
    
    float d = length(lightVec);
    
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;
        
        float ndotl = max(0.0, dot(lightVec, normalWorld));
        float3 lightStrength = ndotl * L.irRadiance;
        
        float att = CalcAttenuation(L.fallOffStart, L.fallOffEnd, d);
        
        lightStrength *= att;
        
        float spotPower = pow(max(0.0, dot(lightVec, normalize(-L.direction))), L.spotPower);
        
        lightStrength *= spotPower;
        
        return BlinnPhong(albedo, L, lightVec, lightStrength, toEye, normalWorld);
    }
}