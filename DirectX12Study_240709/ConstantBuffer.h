#pragma once

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

__declspec(align(256)) struct MeshConsts
{
    Matrix world;
    Matrix worldIT;
};

#define MAX_LIGHTS        3
#define DIRECTIONAL_LIGHT 0x01
#define POINT_LIGHT       0x02
#define SPOT_LIGHT        0x04
#define LIGHT_OFF         0x00

struct Light
{
    Vector3 direction  = Vector3(0.0f, 0.0f, 1.0f);
    float shininess    = 250.0f;
    Vector3 position   = Vector3(0.0f, 0.0f, -5.0f);
    float spotPower    = 1.0f;
    Vector3 irRadiance = Vector3(1.0f);
    float fallOffStart = 0.0f;
    float fallOffEnd   = 1000.0f;
    uint8_t type;
    Vector2 dummy;
};

__declspec(align(256)) struct GlobalConsts
{
    Matrix view;
    Matrix viewInv;
    Matrix proj;
    Matrix projInv;
    Matrix viewProjInv;
    Vector3 eyeWorld;
    float dt;

    Light lights[MAX_LIGHTS];
};

__declspec(align(256)) struct MaterialConsts
{
    Vector3 ambient  = Vector3(0.0f, 0.0f, 0.0f);
    uint32_t texIdx  = 0;
    Vector3 diffuse  = Vector3(0.0f, 0.0f, 0.0f);
    uint32_t texFlag = 0;
    Vector3 specular = Vector3(0.0f, 0.0f, 0.0f);
    uint32_t texNum  = 0;
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
            SAFE_RELEASE(m_uploadBuffer);
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

    ID3D12Resource *GetResource()
    {
        return m_uploadBuffer;
    }

    void Upload(int idx, void *data)
    {
        memcpy(&m_mappedData[idx * m_bufferSize], data, sizeof(T_CONST));
    }

  private:
    uint32_t m_bufferSize          = 0;
    ID3D12Resource *m_uploadBuffer = nullptr;
    uint8_t *m_mappedData          = nullptr;
};
