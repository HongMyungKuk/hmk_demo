#pragma once

using namespace DirectX;

namespace Graphics
{

extern D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
extern std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;

extern std::vector<D3D12_INPUT_ELEMENT_DESC> basicILDesc;

extern ID3DBlob *basicVS;
extern ID3DBlob *basicPS;

extern ID3D12PipelineState *defaultPSO;

void InitGraphicsCommon(ID3D12Device *device, ID3D12RootSignature *rootSignature);
void InitSamplers();
void InitShader();
void InitPipeLineState(ID3D12Device *device, ID3D12RootSignature *rootSignature);

} // namespace Graphics
