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

cbuffer MeshConsts : register(b0)
{
    Matrix world;
    Matrix view;
    Matrix projection;
};

cbuffer MaterialConstants : register(b1)
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

SamplerState linearWrapSS : register(s0);
Texture2D albedoTexture : register(t0);

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
    return albedoTexture.Sample(linearWrapSS, input.texCoord);
}