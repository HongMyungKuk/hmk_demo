#include "pch.h"

#include "GraphicsCommon.h"

namespace Graphics
{
D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

D3D12_RASTERIZER_DESC solidCW;
D3D12_RASTERIZER_DESC wireCW;

std::vector<D3D12_INPUT_ELEMENT_DESC> basicILDesc;
std::vector<D3D12_INPUT_ELEMENT_DESC> normalILDesc;

ID3DBlob *basicVS;
ID3DBlob *basicPS;
ID3DBlob *normalVS;
ID3DBlob *normalGS;
ID3DBlob *normalPS;

ID3D12PipelineState *defaultSolidPSO;
ID3D12PipelineState *defaultWirePSO;
ID3D12PipelineState *normalPSO;

void InitGraphicsCommon(ID3D12Device *device, ID3D12RootSignature *rootSignature)
{
    InitSamplers();
    InitRasterizerState();
    InitShader();
    InitPipeLineState(device, rootSignature);
}

void InitSamplers()
{
    slinearWrapSamplerDesc.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    slinearWrapSamplerDesc.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    slinearWrapSamplerDesc.MipLODBias       = 0;
    slinearWrapSamplerDesc.MaxAnisotropy    = 0;
    slinearWrapSamplerDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    slinearWrapSamplerDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    slinearWrapSamplerDesc.MinLOD           = 0.0f;
    slinearWrapSamplerDesc.MaxLOD           = D3D12_FLOAT32_MAX;
    slinearWrapSamplerDesc.ShaderRegister   = 0;
    slinearWrapSamplerDesc.RegisterSpace    = 0;
    slinearWrapSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    vecSamplerDesc.push_back(slinearWrapSamplerDesc);
}

void InitShader()
{
#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(
        D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "vsmain", "vs_5_1", compileFlags, 0, &basicVS, nullptr));
    ThrowIfFailed(
        D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "psmain", "ps_5_1", compileFlags, 0, &basicPS, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"NormalShader.hlsl", nullptr, nullptr, "vsmain", "vs_5_1", compileFlags, 0,
                                     &normalVS, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"NormalShader.hlsl", nullptr, nullptr, "gsmain", "gs_5_1", compileFlags, 0,
                                     &normalGS, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"NormalShader.hlsl", nullptr, nullptr, "psmain", "ps_5_1", compileFlags, 0,
                                     &normalPS, nullptr));

    basicILDesc  = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    normalILDesc = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
}

void InitRasterizerState()
{
    solidCW.FillMode              = D3D12_FILL_MODE_SOLID;
    solidCW.CullMode              = D3D12_CULL_MODE_BACK;
    solidCW.FrontCounterClockwise = FALSE;
    solidCW.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
    solidCW.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    solidCW.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    solidCW.DepthClipEnable       = TRUE;
    solidCW.MultisampleEnable     = FALSE;
    solidCW.AntialiasedLineEnable = FALSE;
    solidCW.ForcedSampleCount     = 0;
    solidCW.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    wireCW          = solidCW;
    wireCW.FillMode = D3D12_FILL_MODE_WIREFRAME;
}

void InitPipeLineState(ID3D12Device *device, ID3D12RootSignature *rootSignature)
{
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout                        = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.pRootSignature                     = rootSignature;
    psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(basicVS);
    psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(basicPS);
    psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState                  = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask                         = UINT_MAX;
    psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets                   = 1;
    psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat                          = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count                   = 1;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));

    psoDesc.RasterizerState = wireCW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&defaultWirePSO)));

    psoDesc.InputLayout           = {normalILDesc.data(), UINT(normalILDesc.size())};
    psoDesc.VS                    = CD3DX12_SHADER_BYTECODE(normalVS);
    psoDesc.GS                    = CD3DX12_SHADER_BYTECODE(normalGS);
    psoDesc.PS                    = CD3DX12_SHADER_BYTECODE(normalPS);
    psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&normalPSO)));
}

void DestroyPipeLineState()
{
    SAFE_RELEASE(defaultWirePSO);
    SAFE_RELEASE(defaultSolidPSO);
    SAFE_RELEASE(normalPSO);
}

void DestroyShader()
{
    SAFE_RELEASE(normalPS);
    SAFE_RELEASE(normalGS);
    SAFE_RELEASE(normalVS);
    SAFE_RELEASE(basicPS);
    SAFE_RELEASE(basicVS);
}

void DestroyGraphicsCommon()
{
    DestroyPipeLineState();
    DestroyShader();
}
} // namespace Graphics
