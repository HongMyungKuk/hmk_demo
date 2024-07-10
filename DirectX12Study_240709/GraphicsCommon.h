#pragma once

using namespace DirectX;

namespace Graphics
{
extern D3D12_STATIC_SAMPLER_DESC slinearWrapSamplerDesc;
extern std::vector<D3D12_STATIC_SAMPLER_DESC> vecSamplerDesc;
extern ID3D12RootSignature *defaultRS;

void InitSamplers();
void InitRootSignature(ID3D12Device* device);
void InitPipeLineState(ID3D12Device* device);

} // namespace Graphics
