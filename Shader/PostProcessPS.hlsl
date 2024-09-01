Texture2D tex0 : register(t0);

SamplerState linearWrapSS : register(s0);

cbuffer Consts : register(b0)
{
    float exposure;
    float gamma;
}

struct PixelShaderInput
{
    float4 posModel : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float3 GammaCollection(float3 color)
{
    float alpha = 1.0 / gamma;
    
    float3 ret = pow(color, float3(alpha, alpha, alpha));
    
    return ret;
}

float3 CalcExposure(float3 color)
{
    float3 ret = pow(color, exposure);
    
    return ret;
}

void ToneMapping(in out float3 color)
{
    color = CalcExposure(color);
    
    color = GammaCollection(color);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 color = 0.0;
    
    color = tex0.Sample(linearWrapSS, input.texCoord).xyz;
    
    ToneMapping(color);
    
    return float4(color, 1.0);

}