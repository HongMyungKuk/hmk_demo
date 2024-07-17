#pragma once

class CommandAllocatorPool
{
  public:
    CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
    ~CommandAllocatorPool();

    void Create(ID3D12Device *device);
    void Shutdown();

    ID3D12CommandAllocator *RequestAllocator();

    inline size_t Size()
    {
        return m_allocatorPool.size();
    }

  private:
    std::vector<ID3D12CommandAllocator *> m_allocatorPool;
    ID3D12Device *m_device;
    D3D12_COMMAND_LIST_TYPE m_type;
};
