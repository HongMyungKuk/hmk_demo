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
    float dummy2;
    float3 specular;
    float dummy3;
};

SamplerState linearWrapSS : register(s0);
Texture2D albedoTexture[7] : register(t3);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD)
{
    PSInput result;

    result.position = mul(float4(position, 1.0), world);
    result.position = mul(result.position, view);
    result.position = mul(result.position, projection);

    result.normal = normal;
    result.texCoord = texCoord;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return albedoTexture[texIdx].Sample(linearWrapSS, input.texCoord);

}