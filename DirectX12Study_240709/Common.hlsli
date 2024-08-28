#define MAX_LIGHTS 3
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

Texture2D shadowMap0 : register(t11);
Texture2D shadowMap1 : register(t12);
Texture2D shadowMap2 : register(t13);
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
    float fallOffStart;
    float3 position;
    float fallOffEnd;
    float3 irRadiance;
    float spotPower;
    // shadow matrix
    Matrix view;
    Matrix proj;
    
    uint type;
    float radius;
    float haloRadius;
    float haloStrength;
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
    float envStrength;
    float mipmap;
    float fogStrength;

    float3 haloPosition;
    float haloRadius;
    float haloStrength;
}

cbuffer MeshConsts : register(b1)
{
    Matrix world;
    Matrix worldIT;
    
    uint useHeightMap;
    float heightScale;
    float texCoordScale;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor;
    float metalnessFactor;
    float3 emissionFactor;
    float roughnessFactor;
    uint mapFlags;
};

struct VSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    
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
    float3 tangentWorld : TANGENT;
    float2 texCoord : TEXCOORD;
};

struct PSOutput
{
    float4 pixelColor : SV_Target0;
};

float CalcAttenuation(float fallOffStart, float fallOffEnd, float d)
{
    return saturate((fallOffEnd - d) / (fallOffEnd - fallOffStart));
}
