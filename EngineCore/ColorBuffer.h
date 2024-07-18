#pragma once

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

class ColorBuffer
{
    friend class CommandContext;

  public:
    ColorBuffer(Vector4 clearColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f))
        : m_clearColor(clearColor), m_numMipMaps(0), m_fragmentCount(1), m_sampleCount(1)
    {
        m_rtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_srvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        for (int i = 0; i < _countof(m_uavHandle); ++i)
            m_uavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    // Create a color buffer from a swap chain buffer.  Unordered access is restricted.
    void CreateFromSwapChain(const std::wstring &aname, ID3D12Resource *BaseResource);

    ID3D12Resource* GetResource()
    {
        return m_resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV()
    {
        return m_rtvHandle;
    }

  private:
    Vector4 m_clearColor;
    uint32_t m_numMipMaps;
    uint32_t m_fragmentCount;
    uint32_t m_sampleCount;

    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_uavHandle[12];

    // »ó¼ÓÈ­
    ID3D12Resource *m_resource         = nullptr;
    D3D12_RESOURCE_STATES m_usageState = D3D12_RESOURCE_STATE_PRESENT;

  public:
    void Destroy();
};
