//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

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
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 posProj : SV_POSITION;
    float3 normalWorld : NORMAL;
    float2 texCoord : TEXCOORD;
};

PSInput vsmain(VSInput input)
{
    PSInput output;

    output.posProj = mul(float4(input.posModel, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, projection);

    output.normalWorld = input.normalModel;
    output.texCoord = input.texCoord;
    
    return output;
}

float4 psmain(PSInput input) : SV_TARGET
{
    return texFlag ? albedoTexture[texIdx].Sample(linearWrapSS, input.texCoord) : float4(0.5 * diffuse + 0.5 * specular, 1.0);
}