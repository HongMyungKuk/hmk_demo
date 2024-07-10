#pragma once

__declspec(align(256)) struct MeshConsts
{
    XMMATRIX world;
};

__declspec(align(256)) struct GlobalConsts
{
    XMMATRIX view;
    XMMATRIX projeciton;
    XMFLOAT3 eyeWorld;
};

__declspec(align(256)) struct MaterialConsts
{
    XMFLOAT3 ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 diffuse;
    XMFLOAT3 specular;
};

struct Light
{
    XMFLOAT3 direction;
    XMFLOAT3 position;
    XMFLOAT3 irRadiance;
    float shininess;
    float spotPower;
};

template <typename T_CONST> class ConstantBuffer
{

};
