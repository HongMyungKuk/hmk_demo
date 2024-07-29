#include "pch.h"

#include "GraphicsCommon.h"

namespace Graphics
{
D3D12_STATIC_SAMPLER_DESC linearWrapSD;
D3D12_STATIC_SAMPLER_DESC linearClampSD;
D3D12_STATIC_SAMPLER_DESC pointWrapSD;
D3D12_STATIC_SAMPLER_DESC pointClampSD;
D3D12_STATIC_SAMPLER_DESC shadowPointSD;
std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

D3D12_RASTERIZER_DESC solidCW;
D3D12_RASTERIZER_DESC solidMSSACW;
D3D12_RASTERIZER_DESC solidDepthOffCW;
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
ID3DBlob *postProcessPS;

D3D12_BLEND_DESC coverBS;

ID3D12RootSignature *postProcessRootSignature = nullptr;
ID3D12RootSignature *defaultRootSignature     = nullptr;

D3D12_VIEWPORT mainViewport;
D3D12_VIEWPORT shadowViewport;
D3D12_VIEWPORT depthMapViewport;

D3D12_RECT mainSissorRect;
D3D12_RECT shadowSissorRect;

ID3D12PipelineState *defaultSolidPSO;
ID3D12PipelineState *skinnedSolidPSO;
ID3D12PipelineState *defaultWirePSO;
ID3D12PipelineState *skinnedWirePSO;
ID3D12PipelineState *normalPSO;
ID3D12PipelineState *blendCoverPSO;
ID3D12PipelineState *skyboxPSO;
ID3D12PipelineState *depthOnlyPSO;
ID3D12PipelineState *depthOnlySkinnedPSO;
ID3D12PipelineState *depthViewportPSO;
ID3D12PipelineState *postProcessPSO;

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
    // static sampler.
    linearWrapSD.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearWrapSD.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linearWrapSD.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linearWrapSD.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linearWrapSD.MipLODBias       = 0;
    linearWrapSD.MaxAnisotropy    = 0;
    linearWrapSD.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    linearWrapSD.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    linearWrapSD.MinLOD           = 0.0f;
    linearWrapSD.MaxLOD           = D3D12_FLOAT32_MAX;
    linearWrapSD.ShaderRegister   = 0;
    linearWrapSD.RegisterSpace    = 0;
    linearWrapSD.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    vecSamplerDesc.push_back(linearWrapSD);

    linearClampSD.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearClampSD.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearClampSD.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearClampSD.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearClampSD.MipLODBias       = 0;
    linearClampSD.MaxAnisotropy    = 0;
    linearClampSD.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    linearClampSD.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    linearClampSD.MinLOD           = 0.0f;
    linearClampSD.MaxLOD           = D3D12_FLOAT32_MAX;
    linearClampSD.ShaderRegister   = 1;
    linearClampSD.RegisterSpace    = 0;
    linearClampSD.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    vecSamplerDesc.push_back(linearClampSD);

    pointWrapSD                = linearWrapSD;
    pointWrapSD.Filter         = D3D12_FILTER_MIN_MAG_MIP_POINT;
    pointWrapSD.ShaderRegister = 2;
    vecSamplerDesc.push_back(pointWrapSD);

    pointClampSD                = linearClampSD;
    pointClampSD.Filter         = D3D12_FILTER_MIN_MAG_MIP_POINT;
    pointClampSD.ShaderRegister = 3;
    vecSamplerDesc.push_back(pointClampSD);

    shadowPointSD                = linearClampSD;
    shadowPointSD.Filter         = D3D12_FILTER_MIN_MAG_MIP_POINT;
    shadowPointSD.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowPointSD.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowPointSD.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowPointSD.BorderColor    = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    shadowPointSD.MaxAnisotropy  = 0;
    shadowPointSD.ShaderRegister = 4;

    vecSamplerDesc.push_back(shadowPointSD);
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

    D3DUtils::CreateShader(L"PostProcessPS.hlsl", &postProcessPS, "main", "ps_5_1");

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
        rangeObj1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); // t0 ~ t3 Common Texture.
        CD3DX12_DESCRIPTOR_RANGE rangeObj2[1] = {};
        rangeObj2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 10); // t1
        CD3DX12_DESCRIPTOR_RANGE rangeObj3[1] = {};
        rangeObj3[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 15); // t2 t3 t4
        CD3DX12_DESCRIPTOR_RANGE rangeObj4[1] = {};
        rangeObj4[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 5);

        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        CD3DX12_ROOT_PARAMETER rootParameters[8] = {};
        rootParameters[0].InitAsConstantBufferView(0); // b0 : Global Consts
        rootParameters[1].InitAsConstantBufferView(1); // b1 : Mesh Consts
        rootParameters[2].InitAsConstantBufferView(2); // b2 : material Consts
        rootParameters[3].InitAsDescriptorTable(_countof(rangeObj1), rangeObj1, D3D12_SHADER_VISIBILITY_ALL); // t0
        rootParameters[4].InitAsDescriptorTable(_countof(rangeObj2), rangeObj2, D3D12_SHADER_VISIBILITY_ALL); // t1
        rootParameters[5].InitAsDescriptorTable(_countof(rangeObj3), rangeObj3,
                                                D3D12_SHADER_VISIBILITY_ALL); // t2 t3 t4
        rootParameters[6].InitAsDescriptorTable(_countof(rangeObj4), rangeObj4, D3D12_SHADER_VISIBILITY_ALL); // s5
        rootParameters[7].InitAsConstantBufferView(3); // b3 : material Consts

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, UINT(vecSamplerDesc.size()),
                               vecSamplerDesc.data(), rootSignatureFlags);

        ID3DBlob *signature = nullptr;
        ID3DBlob *error     = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (error)
            std::cout << (char *)error->GetBufferPointer() << std::endl;
        ThrowIfFailed(hr);

        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                  IID_PPV_ARGS(&defaultRootSignature)));
    }
    {
        // Create root signature.
        CD3DX12_DESCRIPTOR_RANGE rangeObj1[1] = {};
        rangeObj1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(1, rangeObj1);
        rootParameters[1].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &Graphics::linearWrapSD,
                               rootSignatureFlags);

        ID3DBlob *signature = nullptr;
        ID3DBlob *error     = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (error)
            std::cout << (char *)error->GetBufferPointer() << std::endl;
        ThrowIfFailed(hr);

        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                  IID_PPV_ARGS(&postProcessRootSignature)));
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

    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;
    shadowViewport.Width    = (FLOAT)1024.0f;
    shadowViewport.Height   = (FLOAT)1024.0f;

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

    shadowSissorRect.left   = 0;
    shadowSissorRect.top    = 0;
    shadowSissorRect.right  = 1024;
    shadowSissorRect.bottom = 1024;
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

    solidMSSACW                   = solidCW;
    solidMSSACW.DepthClipEnable   = TRUE;
    solidMSSACW.MultisampleEnable = TRUE;

    solidDepthOffCW                   = solidCW;
    solidDepthOffCW.DepthClipEnable   = FALSE;
    solidDepthOffCW.MultisampleEnable = TRUE;
}

void InitPipeLineState(ID3D12Device *device)
{
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout                        = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.pRootSignature                     = defaultRootSignature;
    psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(basicVS);
    psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(basicPS);
    psoDesc.RasterizerState                    = solidMSSACW;
    psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState                  = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask                         = UINT_MAX;
    psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets                   = 1;
    psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.DSVFormat                          = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count                   = 4;
    psoDesc.SampleDesc.Quality                 = 0;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));

    psoDesc.RasterizerState = wireCW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&defaultWirePSO)));

    psoDesc.InputLayout     = {skinnedILDesc.data(), UINT(skinnedILDesc.size())};
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(skinnedVS);
    psoDesc.RasterizerState = solidMSSACW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skinnedSolidPSO)));

    psoDesc.RasterizerState = wireCW;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skinnedWirePSO)));

    psoDesc.InputLayout     = {skyboxILDesc.data(), UINT(skyboxILDesc.size())};
    psoDesc.RasterizerState = solidMSSACW;
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(skyboxVS);
    psoDesc.PS              = CD3DX12_SHADER_BYTECODE(skyboxPS);
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&skyboxPSO)));

    psoDesc.InputLayout        = {basicILDesc.data(), UINT(basicILDesc.size())};
    psoDesc.RasterizerState    = solidCW;
    psoDesc.VS                 = CD3DX12_SHADER_BYTECODE(basicVS);
    psoDesc.PS                 = CD3DX12_SHADER_BYTECODE(depthOnlyPS);
    psoDesc.DSVFormat          = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count   = 1;
    psoDesc.SampleDesc.Quality = 0;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&depthOnlyPSO)));

    psoDesc.InputLayout     = {skinnedILDesc.data(), UINT(skinnedILDesc.size())};
    psoDesc.RasterizerState = solidCW;
    psoDesc.VS              = CD3DX12_SHADER_BYTECODE(skinnedVS);
    psoDesc.PS              = CD3DX12_SHADER_BYTECODE(depthOnlyPS);
    psoDesc.DSVFormat       = DXGI_FORMAT_D32_FLOAT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&depthOnlySkinnedPSO)));

    psoDesc.InputLayout        = {postEffectsILDesc.data(), UINT(postEffectsILDesc.size())};
    psoDesc.RasterizerState    = solidMSSACW;
    psoDesc.VS                 = CD3DX12_SHADER_BYTECODE(postEffecstVS);
    psoDesc.PS                 = CD3DX12_SHADER_BYTECODE(dummyPS);
    psoDesc.DSVFormat          = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count   = 4;
    psoDesc.SampleDesc.Quality = 0;
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

    psoDesc.pRootSignature        = Graphics::postProcessRootSignature;
    psoDesc.InputLayout           = {postEffectsILDesc.data(), UINT(postEffectsILDesc.size())};
    psoDesc.RasterizerState       = solidDepthOffCW;
    psoDesc.VS                    = CD3DX12_SHADER_BYTECODE(postEffecstVS);
    psoDesc.GS                    = {};
    psoDesc.PS                    = CD3DX12_SHADER_BYTECODE(postProcessPS);
    psoDesc.DSVFormat             = {};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.RTVFormats[0]         = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count      = 1;
    psoDesc.SampleDesc.Quality    = 0;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&postProcessPSO)));
}

void DestroyPipeLineState()
{
    SAFE_RELEASE(postProcessPSO);
    SAFE_RELEASE(depthViewportPSO);
    SAFE_RELEASE(depthOnlySkinnedPSO);
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
    SAFE_RELEASE(postProcessPS);
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
    SAFE_RELEASE(postProcessRootSignature);
    DestroyShader();
}
} // namespace Graphics
