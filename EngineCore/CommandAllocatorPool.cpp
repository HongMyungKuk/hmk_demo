#include "pch.h"

#include "CommandAllocatorPool.h"

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type) : m_device(nullptr), m_type(type)
{
}

CommandAllocatorPool::~CommandAllocatorPool()
{
    Shutdown();
}

void CommandAllocatorPool::Create(ID3D12Device *device)
{
    m_device = device;
}

void CommandAllocatorPool::Shutdown()
{
    for (size_t i = 0; i < m_allocatorPool.size(); i++)
    {
        SAFE_RELEASE(m_allocatorPool[i]);
    }
}

ID3D12CommandAllocator *CommandAllocatorPool::RequestAllocator()
{
    ID3D12CommandAllocator *allocator = nullptr;

    if (!allocator)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&allocator)));
        wchar_t allocatorName[32];
        swprintf(allocatorName, 32, L"CommandAllocator %zu", m_allocatorPool.size());
        allocator->SetName(allocatorName);
        m_allocatorPool.push_back(allocator);
    }

    return allocator;
}
