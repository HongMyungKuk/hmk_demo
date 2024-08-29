#include "pch.h"

#include "FrameResource.h"
#include "AppBase.h"

FrameResource::FrameResource(int numModels, int numLights, ID3D12PipelineState* pso)
{
	ThrowIfFailed(Graphics::g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	m_meshConstsBuffer = new UploadBuffer<MeshConsts>();
	m_materialConstsBuffer = new UploadBuffer<MaterialConsts>();
	m_globalConstsBuffer = new UploadBuffer<GlobalConsts>();
	m_shadowConstsBuffer = new UploadBuffer<GlobalConsts>();
	m_meshConstsBuffer->Initialize(Graphics::g_Device, numModels);
	m_materialConstsBuffer->Initialize(Graphics::g_Device, numModels);
	m_globalConstsBuffer->Initialize(Graphics::g_Device, 1);
	m_shadowConstsBuffer->Initialize(Graphics::g_Device, numLights);
}

FrameResource::~FrameResource()
{
	SAFE_DELETE(m_meshConstsBuffer);
	SAFE_DELETE(m_materialConstsBuffer);
	SAFE_DELETE(m_globalConstsBuffer);
	SAFE_DELETE(m_shadowConstsBuffer);
	SAFE_RELEASE(m_commandAllocator);
}

void FrameResource::Init()
{
}
