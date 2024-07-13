#pragma once

#include "ConstantBuffer.h"
#include "Mesh.h"

using namespace DirectX;

class Model
{
  public:
    Model();
    virtual ~Model();

  public:
    virtual void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                            ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                            std::vector<MeshData> meshes,
                            std::vector<MaterialConsts> materials = {}); // const buffer, mesh data
    virtual void Update();
    virtual void Render(ID3D12GraphicsCommandList *commandList);
    virtual void RenderNormal(ID3D12GraphicsCommandList *commandList);
    void UpdateWorldMatrix(XMMATRIX worldRow);

  private:
    void BuildConstantBufferView(ID3D12Device *device);
    void BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData);
    void BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                      ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                      const std::string &filename, ID3D12Resource **texture, ID3D12Resource **uploadTexture);
    // void BuildDescriptor(ID3D12Device *device);
    void DestroyMeshBuffers();
    void DestroyTextureResource();

  public:
    ID3D12PipelineState *GetPSO(bool isWireFrame)
    {
        return isWireFrame ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO;
    }
    MeshConsts &GetMeshConstCPU()
    {
        return m_meshConstsData;
    }
    MaterialConsts &GetMaterialConstCPU()
    {
        return m_materialConstData;
    }

  private:
    ID3D12RootSignature *m_rootSignature   = nullptr;
    ID3D12PipelineState *m_pipelineState   = nullptr;
    ID3D12Resource *m_meshConstBuffer      = nullptr;
    ID3D12Resource *m_materialConstBuffer  = nullptr;
    ID3D12DescriptorHeap *m_descriptorHeap = nullptr;
    ID3D12Resource *m_textureUploadHeap    = nullptr;
    // uint8_t *m_meshDataBeign                 = nullptr;
    // uint8_t *m_materialDataBeign             = nullptr;
    // MeshConsts m_meshConstBufferData         = {};
    // MaterialConsts m_materialConstBufferData = {};
    std::vector<Mesh> m_meshes             = {};
    std::vector<MaterialConsts> m_material = {};
    uint8_t m_descRef                      = 0;
    uint8_t m_descNum                      = 9;
    uint8_t m_renderRef                    = 0;

    UploadBuffer<MeshConsts> m_meshUpload;
    UploadBuffer<MaterialConsts> m_materialUpload;
    MeshConsts m_meshConstsData        = {};
    MaterialConsts m_materialConstData = {};
};
