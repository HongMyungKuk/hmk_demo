#pragma once

#include "Model.h"

class SkinnedMeshModel : public Model
{
  public:
    void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                    ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                    std::vector<MeshData> meshe, AnimationData anim = {});
    void UpdateAnimation(int clipID, int frameCount);
    virtual void Render(ID3D12GraphicsCommandList *commandList);

  private:
    virtual void BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData) override;

  private:
    UploadBuffer<Matrix> m_boneTransform;
    AnimationData m_anim;
};
