#include "pch.h"

#include "DesciptorHeap.h"
#include "GraphicsCore.h"

DescriptorAllocator::~DescriptorAllocator()
{
    Shutdown();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t count)
{
    // 현재 descripter heap이 없거나, 할당 받을 수 있는 heap 공간이 없을 경우
    if (m_currentHeap == nullptr || m_remainingFreeHandles < count)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Type           = m_type;
        desc.NumDescriptors = sm_NumDescriptorsPerHeap;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask       = 0;
        ThrowIfFailed(Graphics::g_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_currentHeap)));

        m_currentHandle        = m_currentHeap->GetCPUDescriptorHandleForHeapStart();
        m_remainingFreeHandles = sm_NumDescriptorsPerHeap;

        if (m_descriptorSize == 0)
        {
            m_descriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(m_type);
        }
    }

    // 현재의 핸들값을 리턴해준다.
    D3D12_CPU_DESCRIPTOR_HANDLE ret = m_currentHandle;
    // 다음 핸들값의 주소를 업데이트 한다.
    m_currentHandle.ptr += count * m_descriptorSize;
    m_remainingFreeHandles -= count;
    return ret;
}

void DescriptorAllocator::Shutdown()
{
    SAFE_RELEASE(m_currentHeap);
}
