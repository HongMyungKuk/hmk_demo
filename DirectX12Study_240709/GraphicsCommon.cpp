#include "pch.h"

#include "GraphicsCommon.h"

namespace Graphics
{
D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

void InitSamplers()
{
    slinearWrapSamplerDesc.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    slinearWrapSamplerDesc.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.MipLODBias       = 0;
    slinearWrapSamplerDesc.MaxAnisotropy    = 0;
    slinearWrapSamplerDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    slinearWrapSamplerDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    slinearWrapSamplerDesc.MinLOD           = 0.0f;
    slinearWrapSamplerDesc.MaxLOD           = D3D12_FLOAT32_MAX;
    slinearWrapSamplerDesc.ShaderRegister   = 0;
    slinearWrapSamplerDesc.RegisterSpace    = 0;
    slinearWrapSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    vecSamplerDesc.push_back(slinearWrapSamplerDesc);
}

void InitRootSignature(ID3D12Device *device)
{
    CD3DX12_DESCRIPTOR_RANGE rangeObj[2] = {};
    rangeObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // b1 : Mesh Consts, Material, b2 : Material Consts
    rangeObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : Texture

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].InitAsDescriptorTable(_countof(rangeObj), rangeObj, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias                = 0;
    sampler.MaxAnisotropy             = 0;
    sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor               = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD                    = 0.0f;
    sampler.MaxLOD                    = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister            = 0;
    sampler.RegisterSpace             = 0;
    sampler.ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

    ID3DBlob *signature = nullptr;
    ID3DBlob *error     = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                              IID_PPV_ARGS(&defaultRS)));
}

void InitPipeLineState(ID3D12Device *device)
{

}
} // namespace Graphics
