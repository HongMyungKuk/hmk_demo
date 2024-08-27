#include "pch.h"

#include "AppBase.h"
#include "Model.h"

DescriptorHandle s_TerrainSRV;

Model::Model()
{
}

Model::~Model()
{
    DestroyTextureResource();
    DestroyMeshBuffers();

    SAFE_RELEASE(m_pipelineState);
    SAFE_RELEASE(m_rootSignature);
}

void Model::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, std::vector<MeshData> meshes,
                       std::vector<MaterialConsts> materials, bool isTerrian)
{
    m_isTerrian = isTerrian;

    m_meshUpload.Initialize(device, 1);
    m_materialUpload.Initialize(device, 1);

    for (auto &m : meshes)
    {
        Mesh newMesh;
        BuildMeshBuffers(device, newMesh, m);

        // Set Texture
        // 만약 이름이 없다면 더미 텍스쳐를 생성한다. (root desciptor table 설정을 위해 필요)
        {
            this->BuildTexture(device, commandList, m.albedoTextureFilename, &newMesh.albedoTexture,
                               &newMesh.albedoUploadTexture, newMesh.albedoDescriptorHandle, true);
            m_materialConstData.useAlbedoMap = !m.albedoTextureFilename.empty();

            this->BuildTexture(device, commandList, m.metallicTextureFilename, &newMesh.metallicTexture,
                               &newMesh.metallicUploadTexture, newMesh.metallicDescriptorHandle);
            m_materialConstData.useMetalnessMap = !m.metallicTextureFilename.empty();

            this->BuildTexture(device, commandList, m.roughnessTextureFilename, &newMesh.roughnessTexture,
                               &newMesh.roughnessloadTexture, newMesh.roughnessDescriptorHandle);
            m_materialConstData.useRoughnessMap = !m.roughnessTextureFilename.empty();

            this->BuildTexture(device, commandList, m.normalTextureFilename, &newMesh.normalTexture,
                               &newMesh.normalLoadTexture, newMesh.normalDescriptorHandle);
            m_materialConstData.useNormalMap = !m.normalTextureFilename.empty();

            this->BuildTexture(device, commandList, m.heightTextureFilename, &newMesh.heightTexture,
                               &newMesh.heightLoadTexture, newMesh.heightDescriptorHandle);
            m_meshConstsData.useHeightMap = !m.heightTextureFilename.empty();

            this->BuildTexture(device, commandList, m.aoTextureFilename, &newMesh.aoTexture, &newMesh.aoLoadTexture,
                               newMesh.aoDescriptorHandle);
            m_materialConstData.useAoMap = !m.aoTextureFilename.empty();

            this->BuildTexture(device, commandList, m.emissionTextureFilename, &newMesh.emissionTexture,
                               &newMesh.emissionLoadTexture, newMesh.emissionDescriptorHandle);
            m_materialConstData.useEmissiveMap = !m.emissionTextureFilename.empty();
        }

        m_meshes.push_back(newMesh);
    }
}

void Model::Update()
{
    m_meshUpload.Upload(0, &m_meshConstsData);
    m_materialUpload.Upload(0, &m_materialConstData);

    // if (m_material.size() > 0)
    //{
    //     for (int32_t i = 0; i < m_material.size(); i++)
    //     {
    //         m_materialConstData.texIdx   = i;
    //         m_materialConstData.ambient  = m_material[i].ambient;
    //         m_materialConstData.diffuse  = m_material[i].diffuse;
    //         m_materialConstData.specular = m_material[i].specular;
    //         m_materialUpload.Upload(i, &m_materialConstData);
    //     }
    // }
    // else
    //{
    //     m_materialUpload.Upload(0, &m_materialConstData);
    // }
}

void Model::Render(ID3D12GraphicsCommandList *commandList)
{
    int idx = 0;
    for (auto &m : m_meshes)
    {
        if (!m_isTerrian)
            commandList->SetGraphicsRootDescriptorTable(4, m.albedoDescriptorHandle);
        else
            commandList->SetGraphicsRootDescriptorTable(4, s_TerrainSRV);

        commandList->SetGraphicsRootConstantBufferView(1, m_meshUpload.GetResource()->GetGPUVirtualAddress());
        auto address = m_materialUpload.GetResource()->GetGPUVirtualAddress() + idx * sizeof(MaterialConsts);
        commandList->SetGraphicsRootConstantBufferView(2, m_materialUpload.GetResource()->GetGPUVirtualAddress());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m.VertexBufferView());
        commandList->IASetIndexBuffer(&m.IndexBufferView());
        commandList->DrawIndexedInstanced(m.indexCount, 1, 0, 0, 0);
        idx++;

        m_numRenderTriangles += m.indexCount;
    }

    m_numRenderTriangles /= 3;
}

void Model::RenderNormal(ID3D12GraphicsCommandList *commandList)
{
    for (auto &m : m_meshes)
    {
        commandList->SetGraphicsRootConstantBufferView(1, m_meshUpload.GetResource()->GetGPUVirtualAddress());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        commandList->IASetVertexBuffers(0, 1, &m.VertexBufferView());
        commandList->DrawInstanced(m.vertexCount, 1, 0, 0);
    }
}

void Model::UpdateWorldMatrix(Matrix worldRow)
{
    m_world   = worldRow;
    m_worldIT = worldRow;

    m_worldIT.Translation(Vector3(0.0f));
    m_worldIT = m_worldIT.Invert().Transpose();

    m_meshConstsData.world   = m_world.Transpose();
    m_meshConstsData.worldIT = m_worldIT.Transpose();
}

void Model::BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData)
{
    // Create vertex buffer view
    D3DUtils::CreateDefaultBuffer(device, &mesh.vertexBuffer, meshData.vertices.data(),
                                  uint32_t(meshData.vertices.size() * sizeof(Vertex)));
    D3DUtils::CreateDefaultBuffer(device, &mesh.indexBuffer, meshData.indices.data(),
                                  uint32_t(meshData.indices.size() * sizeof(uint32_t)));
    mesh.vertexCount = uint32_t(meshData.vertices.size());
    mesh.stride      = sizeof(Vertex);
    mesh.indexCount  = uint32_t(meshData.indices.size());
}

void Model::BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, const std::string &filename,
                         ID3D12Resource **texture, ID3D12Resource **uploadTexture, DescriptorHandle &handle,
                         bool isSRGB)
{
    handle         = Graphics::s_Texture.Alloc(1);
    *uploadTexture = D3DUtils::CreateTexture(device, commandList, filename, texture,
                                             D3D12_CPU_DESCRIPTOR_HANDLE(handle), {}, isSRGB);
}

void Model::DestroyMeshBuffers()
{
    for (auto &m : m_meshes)
    {
        SAFE_RELEASE(m.vertexBuffer);
        SAFE_RELEASE(m.indexBuffer);
    }
}

void Model::DestroyTextureResource()
{
    for (auto &m : m_meshes)
    {
        SAFE_RELEASE(m.emissionTexture);
        SAFE_RELEASE(m.emissionLoadTexture);

        SAFE_RELEASE(m.aoTexture);
        SAFE_RELEASE(m.aoLoadTexture);

        SAFE_RELEASE(m.heightTexture);
        SAFE_RELEASE(m.heightLoadTexture);

        SAFE_RELEASE(m.normalTexture);
        SAFE_RELEASE(m.normalLoadTexture);

        SAFE_RELEASE(m.roughnessTexture);
        SAFE_RELEASE(m.roughnessloadTexture);

        SAFE_RELEASE(m.metallicTexture);
        SAFE_RELEASE(m.metallicUploadTexture);

        SAFE_RELEASE(m.albedoTexture);
        SAFE_RELEASE(m.albedoUploadTexture);
    }
}
