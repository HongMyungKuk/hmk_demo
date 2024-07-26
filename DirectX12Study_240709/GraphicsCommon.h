#pragma once

using namespace DirectX;

namespace Display
{
extern uint32_t g_screenWidth;
extern uint32_t g_screenHeight;
} // namespace Display

namespace Graphics
{

extern D3D12_STATIC_SAMPLER_DESC linearWrapSD;
extern D3D12_STATIC_SAMPLER_DESC linearClampSD;
extern D3D12_STATIC_SAMPLER_DESC pointWrapSD;
extern D3D12_STATIC_SAMPLER_DESC pointClampSD;
extern D3D12_STATIC_SAMPLER_DESC shadowPointSD;
extern std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

extern D3D12_RASTERIZER_DESC solidCW;
extern D3D12_RASTERIZER_DESC wireCW;

extern std::vector<D3D12_INPUT_ELEMENT_DESC> basicILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> normalILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> skyboxILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> postEffectsILDesc;

extern ID3DBlob *basicVS;
extern ID3DBlob *skinnedVS;
extern ID3DBlob *skyboxVS;
extern ID3DBlob *uiVS;
extern ID3DBlob *postEffecstVS;
extern ID3DBlob *basicPS;
extern ID3DBlob *normalVS;
extern ID3DBlob *normalGS;
extern ID3DBlob *normalPS;
extern ID3DBlob *skyboxPS;
extern ID3DBlob *depthOnlyPS;
extern ID3DBlob *dummyPS;
extern ID3DBlob *postEffectsPS;

extern D3D12_BLEND_DESC coverBS;

extern ID3D12RootSignature *depthOnlyRootSignature;
extern ID3D12RootSignature *defaultRootSignature;

extern D3D12_VIEWPORT mainViewport;
extern D3D12_VIEWPORT shadowViewport;
extern D3D12_VIEWPORT depthMapViewport;

extern D3D12_RECT mainSissorRect;
extern D3D12_RECT shadowSissorRect;

extern ID3D12PipelineState *defaultSolidPSO;
extern ID3D12PipelineState *skinnedSolidPSO;
extern ID3D12PipelineState *defaultWirePSO;
extern ID3D12PipelineState *skinnedWirePSO;
extern ID3D12PipelineState *normalPSO;
extern ID3D12PipelineState *blendCoverPSO;
extern ID3D12PipelineState *skyboxPSO;
extern ID3D12PipelineState *depthOnlyPSO;
extern ID3D12PipelineState *depthOnlySkinnedPSO;
extern ID3D12PipelineState *depthViewportPSO;


void InitGraphicsCommon(ID3D12Device *device);
void InitSamplers();
void InitRasterizerState();
void InitShader();
void InitBlendState();
void InitRootSignature(ID3D12Device *device);
void InitViewportAndScissorRect();
void InitPipeLineState(ID3D12Device *device);

void DestroyPipeLineState();
void DestroyShader();
void DestroyGraphicsCommon();
} // namespace Graphics
