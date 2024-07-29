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
TextureCube diffuseTexture : register(t1);
TextureCube specularTexture : register(t2);
Texture2D brdfTexture : register(t3);

Texture2D shadowMap0 : register(t15);
Texture2D shadowMap1 : register(t16);
Texture2D shadowMap2 : register(t17);

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
    
    uint envType;
}

cbuffer MeshConsts : register(b1)
{
    Matrix world;
    Matrix worldIT;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor;
    float metalnessFactor;
    float3 emissionFactor;
    float roughnessFactor;
    uint useAlbedoMap;
    float3 dummy1;
    uint useMetalnessMap;
    float3 dummy2;
    uint useRoughnessMap;
    float3 dummy3;
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
