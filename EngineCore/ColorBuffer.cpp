#include "pch.h"
#include "ColorBuffer.h"
#include "GraphicsCore.h"


void ColorBuffer::CreateFromSwapChain(const std::wstring &aname, ID3D12Resource *res)
{
    m_resource = res;

    m_rtvHandle = Graphics::AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    Graphics::g_Device->CreateRenderTargetView(m_resource, nullptr, m_rtvHandle);
}

void ColorBuffer::Destroy()
{
    SAFE_RELEASE(m_resource);
}
