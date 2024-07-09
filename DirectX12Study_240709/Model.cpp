#include "Model.h"
#include "D3DUtils.h"
#include "d3dx12.h"
#include <d3dcompiler.h>

Model::Model()
{
}

Model::~Model()
{
}

void Model::Initialize(ID3D12Device *device, std::vector<MeshData> meshes)
{
    BuildRootSignature(device);
    BuildShaderAndGraphicsPSO(device);
}

void Model::Update()
{
}

void Model::Render()
{
}

void Model::BuildRootSignature(ID3D12Device *device)
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
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void Model::BuildConstantBufferView(ID3D12Device *device)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors             = 2;
    cbvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

    auto cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle1(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), 0, cbvDescriptorSize);
    D3DUtils::CreateConstantBuffer(device, m_meshConstBuffer, m_meshDataBeign, &m_meshConstBufferData, cbvHandle1);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle2(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), 1, cbvDescriptorSize);
    D3DUtils::CreateConstantBuffer(device, m_materialConstBuffer, m_materialDataBeign, &m_materialConstBufferData,
                                   cbvHandle2);
}
