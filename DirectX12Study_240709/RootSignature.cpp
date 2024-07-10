#include "pch.h"

#include "RootSignature.h"

void RootSignature::InitAsStaicSampler(uint32_t regiser, const D3D12_SAMPLER_DESC &nonStaticSamplerDesc,
                                       D3D12_SHADER_VISIBILITY visibility)
{
    assert(m_countSamplers < m_numSamplers);
    D3D12_STATIC_SAMPLER_DESC &staticSamplerDesc = m_samplerArr[m_numSamplers++];

    staticSamplerDesc.Filter           = nonStaticSamplerDesc.Filter;
    staticSamplerDesc.AddressU         = nonStaticSamplerDesc.AddressU;
    staticSamplerDesc.AddressV         = nonStaticSamplerDesc.AddressV;
    staticSamplerDesc.AddressW         = nonStaticSamplerDesc.AddressW;
    staticSamplerDesc.MipLODBias       = nonStaticSamplerDesc.MipLODBias;
    staticSamplerDesc.MaxAnisotropy    = nonStaticSamplerDesc.MaxAnisotropy;
    staticSamplerDesc.ComparisonFunc   = nonStaticSamplerDesc.ComparisonFunc;
    staticSamplerDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplerDesc.MinLOD           = nonStaticSamplerDesc.MinLOD;
    staticSamplerDesc.MaxLOD           = nonStaticSamplerDesc.MaxLOD;
    staticSamplerDesc.ShaderRegister   = regiser;
    staticSamplerDesc.RegisterSpace    = 0;
    staticSamplerDesc.ShaderVisibility = visibility;
}
