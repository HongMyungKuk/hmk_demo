#include "pch.h"

#include "AppBase.h"
#include "BillboardModel.h"

void BillboardModel::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                                std::vector<Vector4> points, float width)
{
    m_meshUpload.Initialize(device, 1);
    m_materialUpload.Initialize(device, 1);

    D3DUtils::CreateDefaultBuffer(device, &m_mesh.vertexBuffer, points.data(),
                                  uint32_t(points.size() * sizeof(Vector4)));
    m_mesh.vertexCount = uint32_t(points.size());
    m_mesh.stride      = sizeof(Vector4);

    m_billBoardConstData.width = width;

    m_srv       = Graphics::s_Texture.Alloc(1);
    m_uploadTex = D3DUtils::CreateTexture(device, commandList, "../../Asset/shadertoy_fireball.jpg", &m_texture,
                                          D3D12_CPU_DESCRIPTOR_HANDLE(m_srv));
}

void BillboardModel::Update()
{
    m_meshUpload.Upload(0, &m_meshConstsData);
    m_materialUpload.Upload(0, &m_materialConstData);
}

void BillboardModel::Render(ID3D12GraphicsCommandList *commandList)
{
    commandList->SetGraphicsRootConstantBufferView(1, m_meshUpload.GetResource()->GetGPUVirtualAddress());
    // commandList->SetGraphicsRootConstantBufferView(2, m_materialUpload.GetResource()->GetGPUVirtualAddress());

    commandList->SetGraphicsRootDescriptorTable(4, m_srv);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    commandList->IASetVertexBuffers(0, 1, &m_mesh.VertexBufferView());
    commandList->DrawInstanced(m_mesh.vertexCount, 1, 0, 0);
}
