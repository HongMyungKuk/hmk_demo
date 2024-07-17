#include "pch.h"

#include "CommandListManager.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) : m_type(type), m_allocatorPool(type)
{
}

CommandQueue::~CommandQueue()
{
    Shutdown();
}

void CommandQueue::Create(ID3D12Device *device)
{
    assert(device != nullptr);
    assert(!IsReady());
    assert(m_allocatorPool.Size() == 0);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};

    queueDesc.Type     = m_type;
    queueDesc.NodeMask = 1;
    queueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    m_commandQueue->SetName(L"CommandListManager::m_commandQueue");

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fence->SetName(L"CommandListManager::m_fence");
    m_fence->Signal((uint64_t)m_type << 56);

    m_fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    assert(m_fenceEventHandle != nullptr);

    m_allocatorPool.Create(device);
    assert(IsReady());
}

void CommandQueue::Shutdown()
{
    m_allocatorPool.Shutdown();
    if (m_fenceEventHandle)
    {
        CloseHandle(m_fenceEventHandle);
        m_fenceEventHandle = nullptr;
    }
    SAFE_RELEASE(m_fence);
    SAFE_RELEASE(m_commandQueue);
}

ID3D12CommandAllocator *CommandQueue::RequestAlloctor()
{
    uint64_t completeFence = m_fence->GetCompletedValue();

    return m_allocatorPool.RequestAllocator();
}

// CommandList Manager.
CommandListManager::CommandListManager()
    : m_device(nullptr), m_graphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
      m_computeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE), m_copyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{
}

CommandListManager::~CommandListManager()
{
    Shutdown();
}

void CommandListManager::Create(ID3D12Device *device)
{
    assert(device != nullptr);

    m_device = device;

    m_graphicsQueue.Create(device);
    m_computeQueue.Create(device);
    m_copyQueue.Create(device);
}

void CommandListManager::Shutdown()
{
    m_copyQueue.Shutdown();
    m_computeQueue.Shutdown();
    m_graphicsQueue.Shutdown();
}

void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList **cmdList,
                                              ID3D12CommandAllocator **cmdAlloc)
{
    assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        *cmdAlloc = m_graphicsQueue.RequestAlloctor();
        break;
    case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        *cmdAlloc = m_computeQueue.RequestAlloctor();
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        *cmdAlloc = m_copyQueue.RequestAlloctor();
        break;
    }

    ThrowIfFailed(m_device->CreateCommandList(1, type, *cmdAlloc, nullptr, IID_PPV_ARGS(cmdList)));
    (*cmdList)->SetName(L"CommandList");
}
