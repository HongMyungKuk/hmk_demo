#pragma once

__declspec(align(256)) struct MeshConsts
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

__declspec(align(256)) struct GlobalConsts
{
    XMMATRIX view;
    XMMATRIX projeciton;
    XMFLOAT3 eyeWorld;
};

__declspec(align(256)) struct MaterialConsts
{
    XMFLOAT3 ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 diffuse;
    XMFLOAT3 specular;
};

struct Light
{
    XMFLOAT3 direction;
    XMFLOAT3 position;
    XMFLOAT3 irRadiance;
    float shininess;
    float spotPower;
};

template <typename T_CONST> class UploadBuffer
{
  public:
    UploadBuffer()
    {
    }
    ~UploadBuffer()
    {
        if (m_uploadBuffer)
        {
            m_uploadBuffer->Unmap(0, nullptr);
        }
        m_mappedData = nullptr;
    }
    void Initialize(ID3D12Device *device, uint32_t elementCount)
    {
        m_bufferSize = sizeof(T_CONST);

        ThrowIfFailed(
            device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                            &CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize * elementCount),
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadBuffer)));

        ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void **>(&m_mappedData)));
    }

    ID3D12Resource* GetResource()
    {
        return m_uploadBuffer;
    }

    void Upload(int idx, const T_CONST data)
    {
        memcpy(m_mappedData, &data, sizeof(T_CONST));
    }

  private:
    uint32_t m_bufferSize          = 0;
    ID3D12Resource *m_uploadBuffer = nullptr;
    uint8_t *m_mappedData          = nullptr;
};
