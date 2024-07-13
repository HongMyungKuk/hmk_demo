#define MAX_LIGHTS 3

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
    Matrix projection;
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

SamplerState linearWrapSS : register(s0);
Texture2D albedoTexture[7] : register(t3);

struct VSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
};

struct GSInput
{
    float4 posModel : SV_POSITION;
    float3 normalModel : NORMAL;
};

struct PSInput
{
    float4 posProj : SV_POSITION;
    float3 color : COLOR;
};

GSInput vsmain(VSInput input)
{
    GSInput output;

    output.posModel = float4(input.posModel, 1.0);
    output.normalModel = input.normalModel;
    
    return output;
}

[maxvertexcount(2)]
void gsmain(
	point GSInput input[1],
	inout LineStream<PSInput> outputStream
)
{
    PSInput output;
    
    float4 posWorld = mul(input[0].posModel, world);
    float3 normalWorld = normalize(mul(float4(input[0].normalModel, 0.0), worldIT)).xyz;
    
    // Ç¥¸éÀÇ ÁÂÇ¥
    output.posProj = mul(posWorld, view);
    output.posProj = mul(output.posProj, projection);
    output.color = float3(1.0, 0.0, 0.0);
    
    outputStream.Append(output);
    
    posWorld = float4(posWorld.xyz + normalWorld * 0.05, 1.0);
    // ³ë¸»º¤ÅÍ ³¡Á¡ÀÇ ÁÂÇ¥
    output.posProj = mul(posWorld, view);
    output.posProj = mul(output.posProj, projection);
    output.color = float3(1.0, 1.0, 0.0);
    
    outputStream.Append(output);
}

float4 psmain(PSInput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}