#include "pch.h"

#include "AppBase.h"
#include "GeometryGenerator.h"
#include "PostProcess.h"

void PostProcess::Initialize()
{
    MeshData square = GeometryGenerator::MakeSquare(2.0f, 2.0f);

    D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.vertexBuffer, square.vertices.data(),
                                  square.vertices.size() * sizeof(Vertex));
    D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.indexBuffer, square.indices.data(),
                                  square.indices.size() * sizeof(MeshData::index_t));
    mesh.vertexCount = uint32_t(square.vertices.size());
    mesh.indexCount  = uint32_t(square.indices.size());
    mesh.stride      = sizeof(Vertex);

    m_constsBuffer.Initialize(Graphics::g_Device, 1);
}

void PostProcess::Update()
{
    m_constsBuffer.Upload(0, &m_constsData);
}

void PostProcess::Render(ID3D12GraphicsCommandList *commandList)
{
    commandList->SetGraphicsRootConstantBufferView(1, m_constsBuffer.GetResource()->GetGPUVirtualAddress());

    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView());
    commandList->IASetIndexBuffer(&mesh.IndexBufferView());
    commandList->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
}
