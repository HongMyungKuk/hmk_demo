#include "pch.h"

#include "SkinnedMeshModel.h"

void SkinnedMeshModel::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                                  std::vector<MeshData> meshes, std::vector<MaterialConsts> material,
                                  AnimationData anim)
{
    if (!anim.clips.empty())
    {
        m_anim = anim;

        m_boneTransform.Initialize(device, uint32_t(m_anim.clips.front().keys.size()));

        Matrix m = Matrix();
        for (size_t i = 0; i < m_anim.clips.front().keys.size(); i++)
        {
            m_boneTransform.Upload(i, (void *)&m);
        }
    }

    Model::Initialize(device, commandList, meshes, material);
}

void SkinnedMeshModel::UpdateAnimation(int clipID, int frameCount)
{
    m_anim.Update(clipID, frameCount);

    for (size_t i = 0; i < m_anim.clips.front().keys.size(); i++)
    {
        Matrix m = m_anim.Get(clipID, i, frameCount).Transpose();
        m_boneTransform.Upload(i, &m);
    }
}

void SkinnedMeshModel::Render(ID3D12GraphicsCommandList *commandList)
{
    commandList->SetGraphicsRootConstantBufferView(5, m_boneTransform.GetResource()->GetGPUVirtualAddress());

    Model::Render(commandList);
}

void SkinnedMeshModel::BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData)
{
    // Create vertex buffer view
    D3DUtils::CreateDefaultBuffer(device, &mesh.vertexBuffer, meshData.skinnedVertices.data(),
                                  uint32_t(meshData.skinnedVertices.size() * sizeof(SkinnedVertex)));
    D3DUtils::CreateDefaultBuffer(device, &mesh.indexBuffer, meshData.indices.data(),
                                  uint32_t(meshData.indices.size() * sizeof(uint32_t)));
    mesh.vertexCount = uint32_t(meshData.skinnedVertices.size());
    mesh.stride      = sizeof(SkinnedVertex);
    mesh.indexCount  = uint32_t(meshData.indices.size());
}
