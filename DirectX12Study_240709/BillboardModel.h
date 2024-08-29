#pragma once

#include "ConstantBuffer.h"
#include "Model.h"

class BillboardModel : public Model
{
    struct BliiboardConsts
    {
        float width;
    };

  public:
    ~BillboardModel()
    {
        SAFE_RELEASE(m_uploadTex);
        SAFE_RELEASE(m_texture);
        SAFE_RELEASE(m_mesh.vertexBuffer);
    }

    void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        std::vector<Vector4> points, float width, bool useFrameResource = true);

    virtual void Update(UploadBuffer<MeshConsts>* meshGPU, UploadBuffer<MaterialConsts>* materialGPU);
    virtual void Render(ID3D12GraphicsCommandList *commandList);

    virtual ID3D12PipelineState *GetPSO(bool isWireFrame)
    {
        return Graphics::billBoardPointsPSO;
    }

    virtual ID3D12PipelineState *GetDepthOnlyPSO()
    {
        return Graphics::depthOnlyBillboardPSO;
    }

  private:
    UploadBuffer<BliiboardConsts> m_billBoardUpload;
    BliiboardConsts m_billBoardConstData;
    Mesh m_mesh;

    ID3D12Resource *m_texture = nullptr;
    ID3D12Resource *m_uploadTex = nullptr;

    DescriptorHandle m_srv;
};
