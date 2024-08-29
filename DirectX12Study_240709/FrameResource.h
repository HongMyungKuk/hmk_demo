#pragma once

#include "ConstantBuffer.h"

class FrameResource
{
public:
	FrameResource(int numModels, int numLights = 3, ID3D12PipelineState* pso = nullptr);
	~FrameResource();

	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12PipelineState* m_defaultPSO = nullptr;
	uint64_t m_fence = 0;

	UploadBuffer<MeshConsts>* m_meshConstsBuffer = nullptr;
	UploadBuffer<MaterialConsts>* m_materialConstsBuffer = nullptr;
	UploadBuffer<GlobalConsts>* m_globalConstsBuffer = nullptr;
	UploadBuffer<GlobalConsts>* m_shadowConstsBuffer = nullptr;

	void Init();
};

