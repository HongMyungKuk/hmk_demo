#include "pch.h"

#include "GraphicsCommon.h"

namespace Graphics
{
D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

D3D12_RASTERIZER_DESC solidCW;
D3D12_RASTERIZER_DESC wireCW;

std::vector<D3D12_INPUT_ELEMENT_DESC> basicILDesc;
std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedILDesc;
std::vector<D3D12_INPUT_ELEMENT_DESC> normalILDesc;
std::vector<D3D12_INPUT_ELEMENT_DESC> skyboxILDesc;
std::vector<D3D12_INPUT_ELEMENT_DESC> postEffectsILDesc;

ID3DBlob *basicVS;
ID3DBlob *skinnedVS;
ID3DBlob *skyboxVS;
ID3DBlob *uiVS;
ID3DBlob *postEffecstVS;
ID3DBlob *basicPS;
ID3DBlob *normalVS;
ID3DBlob *normalGS;
ID3DBlob *normalPS;
ID3DBlob *skyboxPS;
ID3DBlob *depthOnlyPS;
ID3DBlob *dummyPS;
ID3DBlob *postEffectsPS;

D3D12_BLEND_DESC coverBS;

ID3D12RootSignature *depthOnlyRootSignature = nullptr;
ID3D12RootSignature *defaultRootSignature   = nullptr;

D3D12_VIEWPORT mainViewport;
D3D12_VIEWPORT depthMapViewport;

D3D12_RECT mainSissorRect;

ID3D12PipelineState *defaultSolidPSO;
ID3D12PipelineState *skinnedSolidPSO;
ID3D12PipelineState *defaultWirePSO;
ID3D12PipelineState *skinnedWirePSO;
ID3D12PipelineState *normalPSO;
ID3D12PipelineState *blendCoverPSO;
ID3D12PipelineState *skyboxPSO;
ID3D12PipelineState *depthOnlyPSO;
ID3D12PipelineState *depthViewportPSO;

void InitGraphicsCommon(ID3D12Device *device)
{
    InitSamplers();
    InitRasterizerState();
    InitShader();
    InitBlendState();
    InitRootSignature(device);
    InitViewportAndScissorRect();
    InitPipeLineState(device);
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

    D3DUtils::CreateShader(L"DefaultVS.hlsl", &basicVS, "main", "vs_5_1");

    D3DUtils::CreateShader(L"DefaultVS.hlsl", &skinnedVS, "main", "vs_5_1", {{"SKINNED", "1"}, {NULL, NULL}});

    D3DUtils::CreateShader(L"UIShader.hlsl", &uiVS, "vsmain", "vs_5_1");

    D3DUtils::CreateShader(L"PostEffectsVS.hlsl", &postEffecstVS, "main", "vs_5_1");

    D3DUtils::CreateShader(L"DepthOnlyPS.hlsl", &depthOnlyPS, "main", "ps_5_1");

    D3DUtils::CreateShader(L"DefaultPS.hlsl", &basicPS, "main", "ps_5_1");

    D3DUtils::CreateShader(L"NormalShader.hlsl", &normalVS, "vsmain", "vs_5_1");

    D3DUtils::CreateShader(L"NormalShader.hlsl", &normalGS, "gsmain", "gs_5_1");

    D3DUtils::CreateShader(L"NormalShader.hlsl", &normalPS, "psmain", "ps_5_1");

    D3DUtils::CreateShader(L"Skybox.hlsl", &skyboxVS, "vsmain", "vs_5_1");

    D3DUtils::CreateShader(L"Skybox.hlsl", &skyboxPS, "psmain", "ps_5_1");

    D3DUtils::CreateShader(L"DummyPS.hlsl", &dummyPS, "main", "ps_5_1");

    D3DUtils::CreateShader(L"PostEffectsPS.hlsl", &postEffectsPS, "main", "ps_5_1");

    basicILDesc = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                   {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                   {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    skinnedILDesc = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 68, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    normalILDesc = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    skyboxILDesc = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    postEffectsILDesc = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
}

void InitBlendState()
{
    coverBS.AlphaToCoverageEnable                 = true;
    coverBS.IndependentBlendEnable                = false;
    coverBS.RenderTarget[0].BlendEnable           = true;
    coverBS.RenderTarget[0].LogicOpEnable         = false;
    coverBS.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;
    coverBS.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
    coverBS.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    coverBS.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    coverBS.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ONE;
    coverBS.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    coverBS.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
    coverBS.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void InitRootSignature(ID3D12Device *device)
{
    {
        // Create root signature.
        CD3DX12_DESCRIPTOR_RANGE rangeObj1[1] = {};
        rangeObj1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0: envTex, t1 ~ 299 : map texture
        CD3DX12_DESCRIPTOR_RANGE rangeObj2[1] = {};
        rangeObj2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1

        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        CD3DX12_ROOT_PARAMETER rootParameters[6] = {};
        rootParameters[0].InitAsConstantBufferView(0); // b0 : Global Consts
        rootParameters[1].InitAsConstantBufferView(1); // b1 : Mesh Consts
        rootParameters[2].InitAsConstantBufferView(2); // b2 : material Consts
        rootParameters[3].InitAsDescriptorTable(_countof(rangeObj1), rangeObj1, D3D12_SHADER_VISIBILITY_ALL); // t0
        rootParameters[4].InitAsDescriptorTable(_countof(rangeObj2), rangeObj2, D3D12_SHADER_VISIBILITY_ALL); // t1
        rootParameters[5].InitAsConstantBufferView(3); // b3 : material Consts

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
        ThrowIfFailed(
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                  IID_PPV_ARGS(&defaultRootSignature)));
    }
    {
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
        rootParameters[0].InitAsShaderResourceView(0); // t0 : depth map.

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

        ID3DBlob *signature = nullptr;
        ID3DBlob *error     = nullptr;
        ThrowIfFailed(
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                  IID_PPV_ARGS(&depthOnlyRootSignature)));
    }
}

void InitViewportAndScissorRect()
{
    mainViewport.TopLeftX = 0;
    mainViewport.TopLeftY = 0;
    mainViewport.MinDepth = 0.0f;
    mainViewport.MaxDepth = 1.0f;
    mainViewport.Width    = (FLOAT)Display::g_screenWidth;
    mainViewport.Height   = (FLOAT)Display::g_screenHeight;

    depthMapViewport.TopLeftX = 0;
    depthMapViewport.TopLeftY = 0;
    depthMapViewport.MinDepth = 0.0f;
    depthMapViewport.MaxDepth = 1.0f;
    depthMapViewport.Width    = (FLOAT)300.0f;
    depthMapViewport.Height   = (FLOAT)200.0f;

    mainSissorRect.left   = 0;
    mainSissorRect.top    = 0;
    mainSissorRect.right  = Display::g_screenWidth;
    mainSissorRect.bottom = Display::g_screenHeight;
}

void InitRasterizerState()
{
    solidCW.FillMode              = D3D12_FILL_MODE_SOLID;
    solidCW.CullMode              = D3D12_CULL_MODE_NONE;
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

void InitPipeLineState(ID3D12Device *device)
{
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout                        = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.pRootSignature                     = defaultRootSignature;
    psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(basicVS);
    psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(basicPS);
    psoDesc.RasterizerState                    = solidCW;
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

    psoDesc.InputLayout     = {skinnedILDesc.data(), UINT(skinnedILDesc.size())};
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(skinnedVS);
    psoDesc.RasterizerState = solidCW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skinnedSolidPSO)));

    psoDesc.RasterizerState = wireCW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skinnedWirePSO)));

    psoDesc.InputLayout     = {skyboxILDesc.data(), UINT(skyboxILDesc.size())};
    psoDesc.RasterizerState = solidCW;
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(skyboxVS);
    psoDesc.PS              = CD3DX12_SHADER_BYTECODE(skyboxPS);
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skyboxPSO)));

    psoDesc.InputLayout     = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.RasterizerState = solidCW;
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(basicVS);
    psoDesc.PS              = CD3DX12_SHADER_BYTECODE(depthOnlyPS);
    psoDesc.DSVFormat       = DXGI_FORMAT_D32_FLOAT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&depthOnlyPSO)));

    psoDesc.InputLayout     = {postEffectsILDesc.data(), UINT(postEffectsILDesc.size())};
    psoDesc.RasterizerState = solidCW;
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(postEffecstVS);
    psoDesc.PS              = CD3DX12_SHADER_BYTECODE(dummyPS);
    psoDesc.DSVFormat       = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&depthViewportPSO)));

    psoDesc.InputLayout = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.VS          = CD3DX12_SHADER_BYTECODE(uiVS);
    psoDesc.PS          = CD3DX12_SHADER_BYTECODE(basicPS);
    psoDesc.BlendState  = coverBS;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&blendCoverPSO)));

    psoDesc.InputLayout           = {normalILDesc.data(), UINT(normalILDesc.size())};
    psoDesc.VS                    = CD3DX12_SHADER_BYTECODE(normalVS);
    psoDesc.GS                    = CD3DX12_SHADER_BYTECODE(normalGS);
    psoDesc.PS                    = CD3DX12_SHADER_BYTECODE(normalPS);
    psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&normalPSO)));
}

void DestroyPipeLineState()
{
    SAFE_RELEASE(depthViewportPSO);
    SAFE_RELEASE(depthOnlyPSO);
    SAFE_RELEASE(skyboxPSO);
    SAFE_RELEASE(skinnedWirePSO);
    SAFE_RELEASE(skinnedSolidPSO);
    SAFE_RELEASE(defaultWirePSO);
    SAFE_RELEASE(defaultSolidPSO);
    SAFE_RELEASE(normalPSO);
    SAFE_RELEASE(blendCoverPSO);
}

void DestroyShader()
{
    SAFE_RELEASE(postEffectsPS);
    SAFE_RELEASE(depthOnlyPS);
    SAFE_RELEASE(dummyPS);
    SAFE_RELEASE(skyboxPS);
    SAFE_RELEASE(skyboxVS);
    SAFE_RELEASE(skinnedVS);
    SAFE_RELEASE(uiVS);
    SAFE_RELEASE(postEffecstVS);
    SAFE_RELEASE(normalPS);
    SAFE_RELEASE(normalGS);
    SAFE_RELEASE(normalVS);
    SAFE_RELEASE(basicPS);
    SAFE_RELEASE(basicVS);
}

void DestroyGraphicsCommon()
{
    DestroyPipeLineState();
    SAFE_RELEASE(defaultRootSignature);
    SAFE_RELEASE(depthOnlyRootSignature);
    DestroyShader();
}
} // namespace Graphics
