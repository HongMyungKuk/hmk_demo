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

void Model::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, ID3D12CommandQueue *commandQueue,
                       std::vector<MeshData> meshes)
{
    BuildRootSignature(device);
    BuildShaderAndGraphicsPSO(device);
    BuildConstantBufferView(device);

    for (auto &m : meshes)
    {
        Mesh newMesh;
        BuildMeshBuffers(device, newMesh, m);

        if (!m.albedoTextureFilename.empty())
        {
            this->BuildTexture(device, commandList, commandQueue, m.albedoTextureFilename, &newMesh.albedoTexture,
                               &newMesh.albedoUploadTexture);
        }

        m_meshes.push_back(newMesh);
    }
}

void Model::Update()
{
    memcpy(m_meshDataBeign, &m_meshConstBufferData, sizeof(MeshConsts));
    memcpy(m_materialDataBeign, &m_materialConstBufferData, sizeof(MaterialConsts));
}

void Model::Render(ID3D12GraphicsCommandList *commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature);

    for (auto &m : m_meshes)
    {
        ID3D12DescriptorHeap *descHeaps[] = {m_descriptorHeap};
        commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
        commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m.VertexBufferView());
        commandList->IASetIndexBuffer(&m.IndexBufferView());
        commandList->DrawIndexedInstanced(m.indexCount, 1, 0, 0, 0);
    }
}

void Model::BuildRootSignature(ID3D12Device *device)
{
    CD3DX12_DESCRIPTOR_RANGE rangeObj[2] = {};
    rangeObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // b1 : Mesh Consts, Material, b2 : Material Consts
    rangeObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : Texture

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].InitAsDescriptorTable(_countof(rangeObj), rangeObj, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias                = 0;
    sampler.MaxAnisotropy             = 0;
    sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor               = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD                    = 0.0f;
    sampler.MaxLOD                    = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister            = 0;
    sampler.RegisterSpace             = 0;
    sampler.ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

    ID3DBlob *signature = nullptr;
    ID3DBlob *error     = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                              IID_PPV_ARGS(&m_rootSignature)));
}

void Model::BuildShaderAndGraphicsPSO(ID3D12Device *device)
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
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

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
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void Model::BuildConstantBufferView(ID3D12Device *device)
{
    D3DUtils::CreateDscriptor(device, 3, &m_descriptorHeap);

    auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle1(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0,
                                             cbvDescriptorSize);
    D3DUtils::CreateConstantBuffer(device, &m_meshConstBuffer, &m_meshDataBeign, &m_meshConstBufferData, cbvHandle1);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle2(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1,
                                             cbvDescriptorSize);
    D3DUtils::CreateConstantBuffer(device, &m_materialConstBuffer, &m_materialDataBeign, &m_materialConstBufferData,
                                   cbvHandle2);
}

void Model::BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData)
{
    // Create vertex buffer view
    D3DUtils::CreateDefaultBuffer(device, &mesh.vertexBuffer, meshData.vertices.data(),
                                  uint32_t(meshData.vertices.size() * sizeof(Vertex)));
    D3DUtils::CreateDefaultBuffer(device, &mesh.indexBuffer, meshData.indices.data(),
                                  uint32_t(meshData.indices.size() * sizeof(uint16_t)));
    mesh.vertexCount = uint32_t(meshData.vertices.size());
    mesh.indexCount  = uint32_t(meshData.indices.size());
}

void Model::BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, ID3D12CommandQueue *commandQueue,
                         const std::string &filename, ID3D12Resource **texture, ID3D12Resource **uploadTexture)
{
    auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 2,
                                            cbvDescriptorSize);
    *uploadTexture = D3DUtils::CreateTexture(device, commandList, commandQueue, filename, texture, srvHandle);
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
    }
}
