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
    Graphics::g_Device->CreateRenderTargetView(m_resource, nullptr, m_rtv);
}

void ColorBuffer::Create(D3D12_RESOURCE_DESC desc, bool useMSAA)
{
    if (useMSAA)
    {
        desc.SampleDesc.Count   = 4;
        desc.SampleDesc.Quality = 0;
    }

    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format            = desc.Format;
    const float clearColor[4]    = {0.0f, 0.2f, 0.4f, 1.0f};
    memcpy(clearValue.Color, clearColor, 4 * sizeof(float));

    ThrowIfFailed(Graphics::g_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&m_resource)));

    m_rtv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_srv = Graphics::s_Texture.Alloc(1);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension                 = D3D12_RTV_DIMENSION_TEXTURE2DMS;

    Graphics::g_Device->CreateRenderTargetView(m_resource, &rtvDesc, m_rtv);
    Graphics::g_Device->CreateShaderResourceView(m_resource, nullptr, m_srv);
}

void ColorBuffer::Create(const int w, const int h, const DXGI_FORMAT format)
{
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels           = 1;
    textureDesc.Format              = format;
    textureDesc.Width               = UINT64(w);
    textureDesc.Height              = UINT64(h);
    textureDesc.Flags               = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    textureDesc.DepthOrArraySize    = 1;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(Graphics::g_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&m_resource)));

    m_rtv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_srv = Graphics::s_Texture.Alloc(1);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

    Graphics::g_Device->CreateRenderTargetView(m_resource, nullptr, m_rtv);
    Graphics::g_Device->CreateShaderResourceView(m_resource, nullptr, m_srv);
}
