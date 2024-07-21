#include "pch.h"

#include "GraphicsCore.h"
#include "Model.h"

#pragma comment(lib, "EngineCore.lib")

using namespace Graphics;
using DirectX::SimpleMath::Vector3;

struct Vertex
{
    Vector3 position;
    Vector3 color;
};

Model::Model()
{
}
Model::~Model()
{
    SAFE_RELEASE(m_uploadBuffer);
    SAFE_RELEASE(m_deaultBuffer);
}

void Model::Initialize()
{
    // Create the vertex buffer view.
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5, 0.0f}, {1.0f, 0.0f, 0.0f}},
    };

    const uint32_t vertexBufferSizxe = sizeof(vertices);

    ThrowIfFailed(
        g_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSizxe),
                                          D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_deaultBuffer)));
    ThrowIfFailed(
        g_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSizxe),
                                          D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_uploadBuffer)));

    // Copy the triangle data to the vertex buffer.
    UINT8 *pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertices.data(), vertices.size());
    m_uploadBuffer->Unmap(0, nullptr);

    CommandContext &initContext = CommandContext::Begin();

    // initContext.TransitionResource()
    initContext.m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_deaultBuffer,
                                                                                    D3D12_RESOURCE_STATE_COMMON,
                                                                                    D3D12_RESOURCE_STATE_COPY_DEST));
    initContext.m_cmdList->CopyBufferRegion(m_deaultBuffer, 0, m_uploadBuffer, 0, vertexBufferSizxe);
    initContext.m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_deaultBuffer,
                                                                                    D3D12_RESOURCE_STATE_COPY_DEST,
                                                                                    D3D12_RESOURCE_STATE_GENERIC_READ));
    m_vertexBufferView.BufferLocation = m_deaultBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes    = vertexBufferSizxe;
    m_vertexBufferView.StrideInBytes  = sizeof(Vertex);

    // Create the index buffer view.
    std::vector<uint32_t> indices = {0, 1, 2};

    const uint32_t indexBufferSize = sizeof(vertices);

    ThrowIfFailed(
        g_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
                                          D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_deaultBuffer2)));
    ThrowIfFailed(
        g_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
                                          D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_uploadBuffer2)));

    // Copy the triangle data to the vertex buffer.
    UINT8 *pIndex;
    ThrowIfFailed(m_uploadBuffer2->Map(0, &readRange, reinterpret_cast<void **>(&pIndex)));
    memcpy(pIndex, vertices.data(), vertices.size());
    m_uploadBuffer2->Unmap(0, nullptr);

    // initContext.TransitionResource()
    initContext.m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_deaultBuffer2,
                                                                                    D3D12_RESOURCE_STATE_COMMON,
                                                                                    D3D12_RESOURCE_STATE_COPY_DEST));
    initContext.m_cmdList->CopyBufferRegion(m_deaultBuffer2, 0, m_uploadBuffer2, 0, vertexBufferSizxe);
    initContext.m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_deaultBuffer2,
                                                                                    D3D12_RESOURCE_STATE_COPY_DEST,
                                                                                    D3D12_RESOURCE_STATE_GENERIC_READ));

    m_indexBufferView.BufferLocation = m_deaultBuffer2->GetGPUVirtualAddress();
    m_indexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes    = indexBufferSize;


    // create the root signature & shader & pso

}

void Model::Render(GraphicsContext &context)
{
    CommandContext &initContext = CommandContext::Begin();

    initContext.m_cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    initContext.m_cmdList->IASetIndexBuffer(&m_indexBufferView);
    initContext.m_cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    initContext.m_cmdList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}
