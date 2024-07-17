#pragma once

class DescriptorAllocator
{
  public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type(type), m_currentHeap(nullptr), m_descriptorSize(0)
    {
        m_currentHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }
    ~DescriptorAllocator();

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);
    void Shutdown();

  private:
    static const uint32_t sm_NumDescriptorsPerHeap = 256;

    D3D12_CPU_DESCRIPTOR_HANDLE m_currentHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    ID3D12DescriptorHeap *m_currentHeap;
    uint32_t m_descriptorSize;
    uint32_t m_remainingFreeHandles;
};

class DesciptorHeap
{
};
