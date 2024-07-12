#include "pch.h"

#include "Model.h"

Model::Model()
{
}

Model::~Model()
{
    DestroyTextureResource();
    DestroyMeshBuffers();
    SAFE_RELEASE(m_descriptorHeap);
    SAFE_RELEASE(m_materialConstBuffer);
    SAFE_RELEASE(m_meshConstBuffer);
    SAFE_RELEASE(m_pipelineState);
    SAFE_RELEASE(m_rootSignature);
}

void Model::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                       ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                       std::vector<MeshData> meshes)
{
    D3DUtils::CreateDscriptor(device, m_descNum, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                              D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, &m_descriptorHeap);

    BuildConstantBufferView(device);

    int32_t idx = 0;
    for (auto &m : meshes)
    {
        Mesh newMesh;
        BuildMeshBuffers(device, newMesh, m);

        // Set Material
        // if (true)
        //{
        //    this->BuildTexture(device, commandList, commandAllocator, commandQueue, "", &newMesh.diffuseUploadTexture,
        //                       &newMesh.diffuseUploadTexture);
        //}

        // Set Texture
        if (!m.albedoTextureFilename.empty())
        {
            this->BuildTexture(device, commandList, commandAllocator, commandQueue, m.albedoTextureFilename,
                               &newMesh.albedoTexture, &newMesh.albedoUploadTexture);
        }

        m_meshes.push_back(newMesh);
    }
}

void Model::Update()
{
    memcpy(m_meshDataBeign, &m_meshConstBufferData, sizeof(MeshConsts));
    memcpy(m_materialDataBeign, &m_materialConstBufferData, sizeof(MaterialConsts));
}

void Model::Render(ID3D12Device *device, ID3D12GraphicsCommandList *commandList)
{
    for (auto &m : m_meshes)
    {
        // Describe and create a SRV for the texture.
        auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 2,
                                                cbvDescriptorSize);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels             = 1;
        device->CreateShaderResourceView(m.albedoTexture, &srvDesc, srvHandle);

        ID3D12DescriptorHeap *descHeaps[] = {m_descriptorHeap};
        commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
        commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m.VertexBufferView());
        commandList->IASetIndexBuffer(&m.IndexBufferView());
        commandList->DrawIndexedInstanced(m.indexCount, 1, 0, 0, 0);
    }
}

void Model::UpdateWorldMatrix(XMMATRIX worldRow)
{
    auto world = XMMatrixTranspose(worldRow);
    // auto worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, world));

    m_meshConstBufferData.world = world;
    // m_meshConstBufferData.worldIT = worldIT;
}

void Model::BuildConstantBufferView(ID3D12Device *device)
{
    auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    D3DUtils::CreateConstantBuffer(device, &m_meshConstBuffer, &m_meshDataBeign, &m_meshConstBufferData, cbvHandle);

    cbvHandle.Offset(1, cbvDescriptorSize);
    D3DUtils::CreateConstantBuffer(device, &m_materialConstBuffer, &m_materialDataBeign, &m_materialConstBufferData,
                                   cbvHandle);
}

void Model::BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData)
{
    // Create vertex buffer view
    D3DUtils::CreateDefaultBuffer(device, &mesh.vertexBuffer, meshData.vertices.data(),
                                  uint32_t(meshData.vertices.size() * sizeof(Vertex)));
    D3DUtils::CreateDefaultBuffer(device, &mesh.indexBuffer, meshData.indices.data(),
                                  uint32_t(meshData.indices.size() * sizeof(uint32_t)));
    mesh.vertexCount = uint32_t(meshData.vertices.size());
    mesh.indexCount  = uint32_t(meshData.indices.size());
}

void Model::BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                         ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue,
                         const std::string &filename, ID3D12Resource **texture, ID3D12Resource **uploadTexture)
{
    // assert(m_descRef < m_descNum - 1);
    // auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), ++m_descRef,
    //                                         cbvDescriptorSize);
    *uploadTexture = D3DUtils::CreateTexture(device, commandList, commandAllocator, commandQueue, filename, texture);
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
