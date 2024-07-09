#include "ModelViewer.h"
#include "GeometryGenerator.h"

ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);

    SAFE_RELEASE(m_materialConstBuffer);
    SAFE_RELEASE(m_meshConstBuffer);
    SAFE_RELEASE(m_cbvHeap);
    SAFE_RELEASE(m_indexBuffer);
    SAFE_RELEASE(m_vertexBuffer);
    SAFE_RELEASE(m_pipelineState);
    SAFE_RELEASE(m_rootSignature);
}

bool ModelViewer::Initialize()
{
    if (!AppBase::Initialize())
    {
        return false;
    }

    // Create an empty root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE rangeObj[1] = {};
        rangeObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // b0, b1

        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangeObj), rangeObj, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

        ID3DBlob *signature = nullptr;
        ID3DBlob *error     = nullptr;
        ThrowIfFailed(
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                    IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ID3DBlob *vertexShader = nullptr;
        ID3DBlob *pixelShader  = nullptr;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0,
                                         &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0,
                                         &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout                        = {inputElementDescs, _countof(inputElementDescs)};
        psoDesc.pRootSignature                     = m_rootSignature;
        psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(vertexShader);
        psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(pixelShader);
        psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable      = FALSE;
        psoDesc.DepthStencilState.StencilEnable    = FALSE;
        psoDesc.SampleMask                         = UINT_MAX;
        psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets                   = 1;
        psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count                   = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    {
        const auto meshData = GeometryGenerator::MakeCube(1.0f, 1.0f, 1.0f);
        // Create the vertex buffer.
        {
            const UINT vertexBufferSize = meshData.vertices.size() * sizeof(Vertex);

            // Note: using upload heaps to transfer static data like vert buffers is not
            // recommended. Every time the GPU needs it, the upload heap will be marshalled
            // over. Please read up on Default Heap usage. An upload heap is used here for
            // code simplicity and because there are very few verts to actually transfer.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_vertexBuffer)));

            // Copy the triangle data to the vertex buffer.
            UINT8 *pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, meshData.vertices.data(), meshData.vertices.size() * sizeof(Vertex));
            m_vertexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes  = sizeof(Vertex);
            m_vertexBufferView.SizeInBytes    = vertexBufferSize;
        }

        // Create the index Buffer.
        {
            const UINT indexBufferSize = meshData.indices.size() * sizeof(uint16_t);

            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_indexBuffer)));

            // Copy the triangle data to the vertex buffer.
            UINT8 *pIndexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin)));
            memcpy(pIndexDataBegin, meshData.indices.data(), meshData.indices.size() * sizeof(uint16_t));
            m_indexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
            m_indexBufferView.Format         = DXGI_FORMAT_R16_UINT;
            m_indexBufferView.SizeInBytes    = indexBufferSize;
        }
    }

    // Create constant buffer.
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors             = 2;
        cbvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

        auto cbvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        {
            const uint32_t constBufferSize = sizeof(MeshConsts);

            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(constBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_meshConstBuffer)));

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvBufferDesc = {};
            cbvBufferDesc.BufferLocation                  = m_meshConstBuffer->GetGPUVirtualAddress();
            cbvBufferDesc.SizeInBytes                     = constBufferSize;
            m_device->CreateConstantBufferView(&cbvBufferDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

            // Map and initialize the constant buffer. We don't unmap this until the
            // app closes. Keeping things mapped for the lifetime of the resource is okay.
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_meshConstBuffer->Map(0, &readRange, reinterpret_cast<void **>(&m_meshDataBeign)));
            memcpy(m_meshDataBeign, &m_meshConstBufferData, sizeof(m_meshConstBufferData));
        }

        {
            const uint32_t constBufferSize = sizeof(MaterialConsts);

            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(constBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_materialConstBuffer)));

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvBufferDesc = {};
            cbvBufferDesc.BufferLocation                  = m_materialConstBuffer->GetGPUVirtualAddress();
            cbvBufferDesc.SizeInBytes                     = constBufferSize;

            CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), 1,
                                                    cbvDescriptorSize);
            m_device->CreateConstantBufferView(&cbvBufferDesc, cbvHandle);

            // Map and initialize the constant buffer. We don't unmap this until the
            // app closes. Keeping things mapped for the lifetime of the resource is okay.
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_materialConstBuffer->Map(0, &readRange, reinterpret_cast<void **>(&m_materialDataBeign)));
            memcpy(m_materialDataBeign, &m_materialConstBufferData, sizeof(m_materialConstBufferData));
        }
    }

    return true;
}

void ModelViewer::Update()
{
    static float dt = 0.0f;
    dt += 1.0f / 10.0f;

    XMFLOAT3 eyePos = XMFLOAT3(0.0f, 0.0f, -5.0f);
    XMFLOAT3 lookTo = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 up     = XMFLOAT3(0.0f, 1.0f, 0.0f);

    auto eyeVec    = XMLoadFloat3(&eyePos);
    auto lookToVec = XMLoadFloat3(&lookTo);
    auto upVec     = XMLoadFloat3(&up);

    m_meshConstBufferData.world = XMMatrixTranspose(XMMatrixRotationY(dt));
    m_meshConstBufferData.view  = XMMatrixTranspose(XMMatrixLookToLH(eyeVec, lookToVec, upVec));
    m_meshConstBufferData.projeciton =
        XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(70.0f), AppBase::GetAspect(), 0.01f, 1000.0f));

    memcpy(m_meshDataBeign, &m_meshConstBufferData, sizeof(m_meshConstBufferData));
}

void ModelViewer::Render()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_pipelineState));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature);

    ID3D12DescriptorHeap *descHeaps[] = {m_cbvHeap};
    m_commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_PRESENT,
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, false, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    // Record commands.
    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
                                         D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);
    m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                            D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();

    m_frameIndex = (m_frameIndex + 1) % s_frameCount;
}
