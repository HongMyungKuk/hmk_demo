#include "pch.h"

#include "FrameResource.h"
#include "AppBase.h"

FrameResource::FrameResource(int numModels, int numLights, DescriptorHandle depthHandle, ID3D12PipelineState* scenePSO, ID3D12PipelineState* shadowPSO)
{
	m_scenePSO = scenePSO;
	m_shadowPSO = shadowPSO;

	for (int i = 0; i < g_NumCommandList; i++)
	{
		ThrowIfFailed(Graphics::g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		ThrowIfFailed(Graphics::g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i], m_scenePSO, IID_PPV_ARGS(&m_commandLists[i])));
		ThrowIfFailed(m_commandLists[i]->Close());
	}

	for (int i = 0; i < g_NumContext; i++)
	{
		ThrowIfFailed(Graphics::g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[i])));
		ThrowIfFailed(Graphics::g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sceneCommandAllocators[i], m_scenePSO, IID_PPV_ARGS(&m_sceneCommandLists[i])));
		
		ThrowIfFailed(Graphics::g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_shadowCommandAllocators[i])));
		ThrowIfFailed(Graphics::g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_shadowCommandAllocators[i], m_shadowPSO, IID_PPV_ARGS(&m_shadowCommandLists[i])));

		ThrowIfFailed(m_sceneCommandLists[i]->Close());
		ThrowIfFailed(m_shadowCommandLists[i]->Close());
	}

	m_meshConstsBuffer = new UploadBuffer<MeshConsts>();
	m_materialConstsBuffer = new UploadBuffer<MaterialConsts>();
	m_globalConstsBuffer = new UploadBuffer<GlobalConsts>();
	m_shadowConstsBuffer = new UploadBuffer<GlobalConsts>();
	m_meshConstsBuffer->Initialize(Graphics::g_Device, numModels);
	m_materialConstsBuffer->Initialize(Graphics::g_Device, numModels);
	m_globalConstsBuffer->Initialize(Graphics::g_Device, 1);
	m_shadowConstsBuffer->Initialize(Graphics::g_Device, numLights);

	m_shadowHandle = depthHandle;

	const uint32_t batchSize = _countof(m_sceneCommandLists) + _countof(m_shadowCommandLists) + 3;
	m_batchSubmit[0] = m_commandLists[CommandListPre];
	memcpy(m_batchSubmit + 1, m_shadowCommandLists, _countof(m_shadowCommandLists) * sizeof(ID3D12CommandList*));
	m_batchSubmit[_countof(m_shadowCommandLists) + 1] = m_commandLists[CommandListMid];
	memcpy(m_batchSubmit + _countof(m_shadowCommandLists) + 2, m_sceneCommandLists, _countof(m_sceneCommandLists) * sizeof(ID3D12CommandList*));
	m_batchSubmit[batchSize - 1] = m_commandLists[CommandListPost];
}

FrameResource::~FrameResource()
{
	SAFE_DELETE(m_meshConstsBuffer);
	SAFE_DELETE(m_materialConstsBuffer);
	SAFE_DELETE(m_globalConstsBuffer);
	SAFE_DELETE(m_shadowConstsBuffer);

	for (int i = 0; i < g_NumCommandList; i++)
	{
		SAFE_RELEASE(m_commandLists[i]);
		SAFE_RELEASE(m_commandAllocators[i]);
	}

	for (int i = 0; i < g_NumContext; i++)
	{
		SAFE_RELEASE(m_sceneCommandLists[i]);
		SAFE_RELEASE(m_sceneCommandAllocators[i]);

		SAFE_RELEASE(m_shadowCommandLists[i]);
		SAFE_RELEASE(m_shadowCommandAllocators[i]);
	}
}

void FrameResource::Init()
{
	// Reset the command allocators and lists for the main thread.
	for (int i = 0; i < CommandListCount; i++)
	{
		ThrowIfFailed(m_commandAllocators[i]->Reset());
		ThrowIfFailed(m_commandLists[i]->Reset(m_commandAllocators[i], m_scenePSO));

		// Clear the depth stencil buffer in preparation for rendering the shadow map.
		m_commandLists[CommandListPre]->ClearDepthStencilView(m_shadowHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}
