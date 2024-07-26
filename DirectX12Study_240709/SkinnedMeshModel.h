#pragma once

#include "Model.h"

class SkinnedMeshModel : public Model
{
  public:
    void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, std::vector<MeshData> meshe,
                    std::vector<MaterialConsts> material, AnimationData anim = {});
    void UpdateAnimation(int clipID, int frameCount);
    virtual void Render(ID3D12GraphicsCommandList *commandList);

    AnimationData GetAnim()
    {
        return m_anim;
    }

    virtual ID3D12PipelineState *GetPSO(bool isWireFrame) override
    {
        return isWireFrame ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
    }

    virtual ID3D12PipelineState *GetDepthOnlyPSO() override
    {
        return Graphics::depthOnlySkinnedPSO;
    }

  private:
    virtual void BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData) override;

  private:
    UploadBuffer<Matrix> m_boneTransform;
    AnimationData m_anim;
};
