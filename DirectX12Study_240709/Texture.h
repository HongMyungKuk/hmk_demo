#pragma once

class Texture
{
  public:
    Texture()
    {
    }

    void Create2D(uint32_t w, uint32_t h, DXGI_FORMAT format, const void *initData);

    const D3D12_CPU_DESCRIPTOR_HANDLE &GetSRV() const
    {
        return m_cpuDescriptorHandle;
    }

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle = {};

    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
