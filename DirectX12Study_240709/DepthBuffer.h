#pragma once

namespace Graphics
{
extern ID3D12Device *g_Device;
}

#include "DescriptorHeap.h"

class DepthBuffer
{
  public:
    DepthBuffer();
    ~DepthBuffer();

    void Create(uint32_t w, uint32_t h, DXGI_FORMAT format, bool depthOnly = false, bool useMSAA = true);

    const D3D12_CPU_DESCRIPTOR_HANDLE &GetDSV()
    {
        return m_dsv;
    }

    const DescriptorHandle &GetSRV()
    {
        return m_srv;
    }

    ID3D12Resource *GetResource()
    {
        return m_resource;
    }

  private:
    ID3D12Resource *m_resource        = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsv = {}; // [0] : backbuffer depth, [1] : depth only
    DescriptorHandle m_srv            = {};
};
