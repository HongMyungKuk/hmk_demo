#pragma once

extern ID3D12Device *g_Device;

class DescriptorAllocator
{
  public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type(type), m_currentHandle{}, m_currentHeap(nullptr)
    {
    }
    ~DescriptorAllocator();

    void Shutdown();
    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);
    ID3D12DescriptorHeap *RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

  private:
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentHandle;
    ID3D12DescriptorHeap *m_currentHeap;

    static const uint32_t sm_numDescriptorPerHeap = 512;
    uint32_t m_remainingFreeHandles               = 0;
    uint32_t m_descriptorSize                     = 0;
};

class DescriptorHandle
{
  public:
    DescriptorHandle()
    {
    }

    DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) : m_CPU(cpu), m_GPU(gpu)
    {
    }

    DescriptorHandle operator+(uint32_t byteSize)
    {
        DescriptorHandle ret = *this;
        ret += byteSize;
        return ret;
    }

    void operator+=(uint32_t byteSize)
    {
        m_CPU.ptr += byteSize;
        m_GPU.ptr += byteSize;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE *operator&()
    {
        return &m_CPU;
    }

    operator D3D12_CPU_DESCRIPTOR_HANDLE() const
    {
        return m_CPU;
    }

    operator D3D12_GPU_DESCRIPTOR_HANDLE() const
    {
        return m_GPU;
    }

    bool IsNULL()
    {
        return !m_CPU.ptr;
    }

    bool IsShaderVisible()
    {
        return m_GPU.ptr;
    }

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_CPU = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPU = {};
};

class DescriptorHeap
{
  public:
    ~DescriptorHeap();

    void Create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Type           = type;
        desc.NumDescriptors = maxCount;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        desc.NodeMask       = 0;
        ThrowIfFailed(g_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap)));

        m_firstHandle       = DescriptorHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                               m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
        m_descriptorSize    = g_Device->GetDescriptorHandleIncrementSize(type);
        m_numFreeDescriptor = desc.NumDescriptors;
        m_secondHandle      = m_firstHandle;
    }

    DescriptorHandle Alloc(uint32_t count)
    {
        DescriptorHandle ret = m_secondHandle;
        m_secondHandle += count * m_descriptorSize;
        m_numFreeDescriptor -= count;
        return ret;
    }

    ID3D12DescriptorHeap *Get()
    {
        return m_descriptorHeap;
    }

    DescriptorHandle FirstHandle()
    {
        return m_firstHandle;
    }

    DescriptorHandle operator[](int idx)
    {
        return m_firstHandle + idx * m_descriptorSize;
    }

  public:
    uint32_t m_descriptorSize              = 0;
    uint32_t m_numFreeDescriptor           = 0;
    ID3D12DescriptorHeap *m_descriptorHeap = nullptr;
    DescriptorHandle m_firstHandle;
    DescriptorHandle m_secondHandle;
};
