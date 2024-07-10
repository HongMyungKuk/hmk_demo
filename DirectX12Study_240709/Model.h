#pragma once

#include "Mesh.h"

using namespace DirectX;

__declspec(align(256)) struct MeshConsts
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projeciton;
};

__declspec(align(256)) struct MaterialConsts
{
    XMFLOAT3 ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 diffuse;
    XMFLOAT3 specular;
};

class Model
{
  public:
    Model();
    virtual ~Model();

  public:
    virtual void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                            ID3D12CommandQueue *commandQueue,
                            std::vector<MeshData> meshes); // const buffer, mesh data
    virtual void Update();
    virtual void Render(ID3D12GraphicsCommandList *commandList);

  private:
    void BuildRootSignature(ID3D12Device *device);
    void BuildShaderAndGraphicsPSO(ID3D12Device *device);
    void BuildConstantBufferView(ID3D12Device *device);
    void BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData);
    void BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, ID3D12CommandQueue *commandQueue,
                      const std::string &filename, ID3D12Resource **texture, ID3D12Resource **uploadTexture);
    void BuildDescriptor(ID3D12Device *device);
    void DestroyMeshBuffers();
    void DestroyTextureResource();

  public:
    ID3D12PipelineState *GetPSO()
    {
        return m_pipelineState;
    }
    MeshConsts &GetMeshConstCPU()
    {
        return m_meshConstBufferData;
    }

  private:
    ID3D12RootSignature *m_rootSignature     = nullptr;
    ID3D12PipelineState *m_pipelineState     = nullptr;
    ID3D12Resource *m_meshConstBuffer        = nullptr;
    ID3D12Resource *m_materialConstBuffer    = nullptr;
    ID3D12DescriptorHeap *m_descriptorHeap   = nullptr;
    ID3D12Resource *m_textureUploadHeap      = nullptr;
    uint8_t *m_meshDataBeign                 = nullptr;
    uint8_t *m_materialDataBeign             = nullptr;
    MeshConsts m_meshConstBufferData         = {};
    MaterialConsts m_materialConstBufferData = {};
    std::vector<Mesh> m_meshes               = {};
};
