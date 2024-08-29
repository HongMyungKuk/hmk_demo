#pragma once

#include "ConstantBuffer.h"

#define SINGLETHREADED FALSE

const int g_NumContext = 3;
const int g_NumCommandList = 3;
const int g_NumFrameResource = 3;

class FrameResource
{
public:
	FrameResource(int numModels, int numLights = 3, ID3D12PipelineState* scenePSO = nullptr, ID3D12PipelineState* shadowPSO = nullptr);
	~FrameResource();

	ID3D12CommandList* m_batchSubmit[g_NumCommandList + g_NumContext] = {};

	ID3D12GraphicsCommandList* m_commandLists[g_NumCommandList] = {};
	ID3D12CommandAllocator* m_commandAllocators[g_NumCommandList] = {};

	ID3D12GraphicsCommandList* m_sceneCommandLists[g_NumContext] = {};
	ID3D12CommandAllocator* m_sceneCommandAllocators[g_NumContext] = {};	

	ID3D12GraphicsCommandList* m_shadowCommandLists[g_NumContext] = {};
	ID3D12CommandAllocator* m_shadowCommandAllocators[g_NumContext] = {};

	ID3D12PipelineState* m_scenePSO = nullptr;
	ID3D12PipelineState* m_shadowPSO = nullptr;

	uint64_t m_fence = 0;

	UploadBuffer<MeshConsts>* m_meshConstsBuffer = nullptr;
	UploadBuffer<MaterialConsts>* m_materialConstsBuffer = nullptr;
	UploadBuffer<GlobalConsts>* m_globalConstsBuffer = nullptr;
	UploadBuffer<GlobalConsts>* m_shadowConstsBuffer = nullptr;

	DescriptorHandle m_shadowHandle;

public:
	void Init();
};

