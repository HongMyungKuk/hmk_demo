#pragma once

class DescriptorAllocator
{
  public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type(type)
    {
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);
    ID3D12DescriptorHeap *RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

  private:
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentHandle;
    ID3D12DescriptorHeap *m_currentHeap;

    static const uint32_t sm_numDescriptorPerHeap = 256;
    uint32_t m_remainingFreeHandles               = 0;
    uint32_t m_descriptorSize                     = 0;
};
