#include "pch.h"

#include "ColorBuffer.h"
#include "AppBase.h"

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

void ColorBuffer::Create()
{

}
