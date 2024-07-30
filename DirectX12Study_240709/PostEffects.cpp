#include "pch.h"

#include "AppBase.h"
#include "GeometryGenerator.h"
#include "PostEffects.h"

void PostEffects::Initialize()
{
    MeshData square = GeometryGenerator::MakeSquare(2.0f, 2.0f);

    D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.vertexBuffer, square.vertices.data(),
                                  uint32_t(square.vertices.size() * sizeof(Vertex)));
    D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.indexBuffer, square.indices.data(),
                                  uint32_t(square.indices.size() * sizeof(MeshData::index_t)));
    mesh.vertexCount = uint32_t(square.vertices.size());
    mesh.indexCount  = uint32_t(square.indices.size());
    mesh.stride      = sizeof(Vertex);
}

void PostEffects::Update(const GlobalConsts &data)
{
}

void PostEffects::Render(ID3D12GraphicsCommandList *commandList)
{
    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView());
    commandList->IASetIndexBuffer(&mesh.IndexBufferView());
    commandList->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
}
