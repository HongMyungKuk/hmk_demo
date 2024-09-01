#include "Common.hlsli"

struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION;
    float3 normalModel : NORMAL;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 color : COLOR;
};

GeometryShaderInput vsmain(VertexShaderInput input)
{
    GeometryShaderInput output;

    output.posModel = float4(input.posModel, 1.0);
    output.normalModel = input.normalModel;
    
    return output;
}

[maxvertexcount(2)]
void gsmain(
	point GeometryShaderInput input[1],
	inout LineStream<PixelShaderInput> outputStream
)
{
    PixelShaderInput output;
    
    float4 posWorld = mul(input[0].posModel, world);
    float3 normalWorld = normalize(mul(float4(input[0].normalModel, 0.0), worldIT)).xyz;
    
    // Ç¥¸éÀÇ ÁÂÇ¥
    output.posProj = mul(posWorld, view);
    output.posProj = mul(output.posProj, proj);
    output.color = float3(1.0, 0.0, 0.0);
    
    outputStream.Append(output);
    
    posWorld = float4(posWorld.xyz + normalWorld * 0.05, 1.0);
    // ³ë¸»º¤ÅÍ ³¡Á¡ÀÇ ÁÂÇ¥
    output.posProj = mul(posWorld, view);
    output.posProj = mul(output.posProj, proj);
    output.color = float3(1.0, 1.0, 0.0);
    
    outputStream.Append(output);
}

float4 psmain(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}