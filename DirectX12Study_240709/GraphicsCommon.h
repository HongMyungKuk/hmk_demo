#pragma once

using namespace DirectX;

namespace Graphics
{

extern D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
extern std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

extern D3D12_RASTERIZER_DESC solidCW;
extern D3D12_RASTERIZER_DESC wireCW;

extern std::vector<D3D12_INPUT_ELEMENT_DESC> basicILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedILDesc;
extern std::vector<D3D12_INPUT_ELEMENT_DESC> normalILDesc;

extern ID3DBlob *basicVS;
extern ID3DBlob *skinnedVS;
extern ID3DBlob *uiVS;
extern ID3DBlob *basicPS;
extern ID3DBlob *normalVS;
extern ID3DBlob *normalGS;
extern ID3DBlob *normalPS;

extern D3D12_BLEND_DESC coverBS;

extern ID3D12PipelineState *defaultSolidPSO;
extern ID3D12PipelineState *skinnedSolidPSO;
extern ID3D12PipelineState *defaultWirePSO;
extern ID3D12PipelineState *skinnedWirePSO;
extern ID3D12PipelineState *normalPSO;
extern ID3D12PipelineState *blendCoverPSO;

void InitGraphicsCommon(ID3D12Device *device, ID3D12RootSignature *rootSignature);
void InitSamplers();
void InitRasterizerState();
void InitShader();
void InitBlendState();
void InitPipeLineState(ID3D12Device *device, ID3D12RootSignature *rootSignature);

void DestroyPipeLineState();
void DestroyShader();
void DestroyGraphicsCommon();
} // namespace Graphics
