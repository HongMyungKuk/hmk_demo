#pragma once

#include "AppBase.h"

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

class ModelViewer : public AppBase
{
  public:
    ModelViewer();
    virtual ~ModelViewer();

    virtual bool Initialize() override;

  protected:
    virtual void Update() override;
    virtual void Render() override;

  private:
    ID3D12RootSignature *m_rootSignature        = nullptr;
    ID3D12PipelineState *m_pipelineState        = nullptr;
    ID3D12Resource *m_vertexBuffer              = nullptr;
    ID3D12Resource *m_indexBuffer               = nullptr;
    ID3D12Resource *m_meshConstBuffer           = nullptr;
    ID3D12Resource *m_materialConstBuffer       = nullptr;
    ID3D12DescriptorHeap *m_cbvHeap             = nullptr;
    uint8_t *m_meshDataBeign                    = nullptr;
    uint8_t *m_materialDataBeign                = nullptr;
    MeshConsts m_meshConstBufferData            = {};
    MaterialConsts m_materialConstBufferData    = {};
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView   = {};
};
