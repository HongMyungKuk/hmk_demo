#define MAX_LIGHTS 3

SamplerState linearWrapSS : register(s0);

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
    float3 position;
    float3 irRadiance;
    float shininess;
    float spotPower;
};

cbuffer GloabalConsts : register(b0)
{
    Matrix view;
    Matrix viewInv;
    Matrix proj;
    Matrix projInv;
    Matrix viewProjInv;
    float3 eyeWorld;

    Light light[MAX_LIGHTS];
}

cbuffer MeshConsts : register(b1)
{
    Matrix world;
    Matrix worldIT;
};

cbuffer MaterialConstants : register(b2)
{
    float3 ambient;
    uint texIdx;
    float3 diffuse;
    uint texFlag;
    float3 specular;
    float dummy3;
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
    float3 normalWorld : NORMAL;
    float2 texCoord : TEXCOORD;
};