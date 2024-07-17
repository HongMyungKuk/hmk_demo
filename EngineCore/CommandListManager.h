#pragma once

#include "CommandAllocatorPool.h"

class CommandQueue
{
  public:
    // CommandQueue();
    CommandQueue(D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();

    void Create(ID3D12Device *device);
    void Shutdown();

    inline bool IsReady()
    {
        return m_commandQueue != nullptr;
    }

    ID3D12CommandQueue *GetCommandQueue()
    {
        return m_commandQueue;
    }

    ID3D12CommandAllocator *RequestAlloctor();

  private:
    const D3D12_COMMAND_LIST_TYPE m_type;

    ID3D12CommandQueue *m_commandQueue = nullptr;
    CommandAllocatorPool m_allocatorPool;

    ID3D12Fence *m_fence      = nullptr;
    HANDLE m_fenceEventHandle = nullptr;
};

class CommandListManager
{
  public:
    CommandListManager();
    ~CommandListManager();

    void Create(ID3D12Device *device);
    void Shutdown();

    void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList **cmdList,
                              ID3D12CommandAllocator **cmdAlloc);

    ID3D12CommandQueue *GetCommandQueue()
    {
        return m_graphicsQueue.GetCommandQueue();
    }

  private:
    ID3D12Device *m_device;

    CommandQueue m_graphicsQueue;
    CommandQueue m_computeQueue;
    CommandQueue m_copyQueue;
};
