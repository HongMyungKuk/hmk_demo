#include "pch.h"

#include "AppBase.h"
#include "ColorBuffer.h"

using namespace Graphics;

ColorBuffer::ColorBuffer()
{
}

ColorBuffer::~ColorBuffer()
{
    SAFE_RELEASE(m_resource);
}

void ColorBuffer::CreateFromSwapChain(ID3D12Resource *resource)
{
    m_resource = resource;

    m_rtv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    g_Device->CreateRenderTargetView(m_resource, nullptr, m_rtv);
}

void ColorBuffer::Create(D3D12_RESOURCE_DESC desc, bool useMSAA)
{
    if (useMSAA)
    {
        desc.SampleDesc.Count   = 4;
        desc.SampleDesc.Quality = 0;
    }

    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    ThrowIfFailed(g_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                    nullptr, IID_PPV_ARGS(&m_resource)));

    m_rtv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_srv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    g_Device->CreateRenderTargetView(m_resource, nullptr, m_rtv);
    g_Device->CreateShaderResourceView(m_resource, nullptr, m_srv);
}

void ColorBuffer::Create(const int w, const int h, const DXGI_FORMAT format)
{
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels           = 1;
    textureDesc.Format              = format;
    textureDesc.Width               = UINT64(w);
    textureDesc.Height              = UINT64(h);
    textureDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize    = 1;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(g_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&m_resource)));
}
