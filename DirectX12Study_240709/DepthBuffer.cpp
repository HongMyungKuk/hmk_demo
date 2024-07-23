#include "pch.h"

#include "AppBase.h"
#include "DepthBuffer.h"

DepthBuffer::DepthBuffer()
{
}

DepthBuffer::~DepthBuffer()
{
    SAFE_RELEASE(m_resource);
}

void DepthBuffer::Create(uint32_t w, uint32_t h, DXGI_FORMAT format, bool depthOnly)
{
    m_dsv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_srv = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_RESOURCE_DESC depthStencilDesc  = {};
    D3D12_CLEAR_VALUE optClear            = {};
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    if (!depthOnly)
    {
        // back buffer depth
        depthStencilDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment          = 0;
        depthStencilDesc.Width              = (UINT64)w;
        depthStencilDesc.Height             = (UINT64)h;
        depthStencilDesc.DepthOrArraySize   = 1;
        depthStencilDesc.MipLevels          = 1;
        depthStencilDesc.Format             = format;
        depthStencilDesc.SampleDesc.Count   = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        optClear.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        optClear.DepthStencil.Depth   = 1.0f;
        optClear.DepthStencil.Stencil = 0;

        ThrowIfFailed(Graphics::g_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_resource)));

        dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.Texture2D.MipSlice = 0;
        Graphics::g_Device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv);
    }
    else
    {
        // depth only
        depthStencilDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment          = 0;
        depthStencilDesc.Width              = (UINT64)w;
        depthStencilDesc.Height             = (UINT64)h;
        depthStencilDesc.DepthOrArraySize   = 1;
        depthStencilDesc.MipLevels          = 1;
        depthStencilDesc.Format             = format;
        depthStencilDesc.SampleDesc.Count   = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format               = DXGI_FORMAT_D32_FLOAT;
        optClear.DepthStencil.Depth   = 1.0f;
        optClear.DepthStencil.Stencil = 0;

        ThrowIfFailed(Graphics::g_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_resource)));

        dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Texture2D.MipSlice = 0;
        Graphics::g_Device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format                          = DXGI_FORMAT_R32_FLOAT;
        srvDesc.Texture2D.MipLevels             = 1;
        srvDesc.Texture2D.MostDetailedMip       = 0;
        Graphics::g_Device->CreateShaderResourceView(m_resource, &srvDesc, m_srv);
    }
}
