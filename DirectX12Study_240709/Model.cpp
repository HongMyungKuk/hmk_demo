#include "pch.h"

#include "AppBase.h"
#include "Model.h"

Model::Model()
{
}

Model::~Model()
{
    DestroyTextureResource();
    DestroyMeshBuffers();

    SAFE_RELEASE(m_pipelineState);
    SAFE_RELEASE(m_objDescriptorHeap);
    SAFE_RELEASE(m_rootSignature);
}

void Model::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                       ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                       std::vector<MeshData> meshes, std::vector<MaterialConsts> materials)
{
    m_meshUpload.Initialize(device, 1);
    m_materialUpload.Initialize(device, uint32_t(meshes.size()));

    int count = 0;
    for (auto &m : meshes)
    {
        Mesh newMesh;
        BuildMeshBuffers(device, newMesh, m);

        // Set Texture
        if (!m.albedoTextureFilename.empty())
        {
            this->BuildTexture(device, commandList, commandAllocator, commandQueue, m.albedoTextureFilename,
                               &newMesh.albedoTexture, &newMesh.albedoUploadTexture);
            count++;
        }

        m_meshes.push_back(newMesh);
    }

    for (auto &m : materials)
    {
        m.texNum = count;
        m_material.push_back(m);
    }
}

void Model::Update()
{
    m_meshUpload.Upload(0, &m_meshConstsData);

    if (m_material.size() > 0)
    {
        for (int32_t i = 0; i < m_material.size(); i++)
        {
            m_materialConstData.texIdx   = i;
            m_materialConstData.ambient  = m_material[i].ambient;
            m_materialConstData.diffuse  = m_material[i].diffuse;
            m_materialConstData.specular = m_material[i].specular;
            m_materialUpload.Upload(i, &m_materialConstData);
        }
    }
    else
    {
        m_materialUpload.Upload(0, &m_materialConstData);
    }
}

void Model::Render(ID3D12GraphicsCommandList *commandList)
{
    //// TODO!!
    //// ���� �ѹ��� ����� ���� ����.
    // ID3D12DescriptorHeap *descHeaps[] = {m_desciptorHeap, m_objDescriptorHeap};
    // commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
    // commandList->SetGraphicsRootDescriptorTable(4, m_desciptorHeap->GetGPUDescriptorHandleForHeapStart());
    // commandList->SetGraphicsRootDescriptorTable(3, m_objDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    int idx = 0;
    for (auto &m : m_meshes)
    {
        auto handle = m_desciptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += g_renderCnt * m_cbvDescriptorSize;

        commandList->SetGraphicsRootDescriptorTable(3, handle);
        commandList->SetGraphicsRootConstantBufferView(1, m_meshUpload.GetResource()->GetGPUVirtualAddress());
        auto address = m_materialUpload.GetResource()->GetGPUVirtualAddress() + idx * sizeof(MaterialConsts);
        commandList->SetGraphicsRootConstantBufferView(2, address);

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m.VertexBufferView());
        commandList->IASetIndexBuffer(&m.IndexBufferView());
        commandList->DrawIndexedInstanced(m.indexCount, 1, 0, 0, 0);

        g_renderCnt++;
        idx++;
    }
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

// ���� ��ü�� �׸� ��� descriptor heap�� �������� ������ �ʿ伺�� ����.
// or �� ��ü�� �׷����� �ؽ����� ī��Ʈ Ƚ���� ����صΰ� ���� ��ü�� �ε����� ������
void Model::BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                         ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                         const std::string &filename, ID3D12Resource **texture, ID3D12Resource **uploadTexture)
{
    m_cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_desciptorHeap->GetCPUDescriptorHandleForHeapStart());
    srvHandle.Offset(g_descCnt, m_cbvDescriptorSize);
    *uploadTexture =
        D3DUtils::CreateTexture(device, commandList, commandAllocator, commandQueue, filename, texture, srvHandle);

    g_descCnt++;
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
        SAFE_RELEASE(m.albedoTexture);
        SAFE_RELEASE(m.albedoUploadTexture);
        SAFE_RELEASE(m.specularUploadTexture);
        SAFE_RELEASE(m.diffuseUploadTexture);
    }
}
