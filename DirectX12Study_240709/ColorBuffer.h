#pragma once

namespace Graphics
{
extern ID3D12Device *g_Device;
}

class ColorBuffer
{
  public:
    ColorBuffer();
    ~ColorBuffer();

    void CreateFromSwapChain(ID3D12Resource *resource);
    
    void Create(D3D12_RESOURCE_DESC desc, bool useMSAA = true);
    void Create(const int w, const int h, const DXGI_FORMAT format);

    const D3D12_CPU_DESCRIPTOR_HANDLE &GetRTV()
    {
        return m_rtv;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE &GetSRV()
    {
        return m_srv;
    }

    ID3D12Resource *GetResource()
    {
        return m_resource;
    }

  private:
    ID3D12Resource *m_resource        = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtv = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_srv = {};
};
