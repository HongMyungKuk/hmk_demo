#include "pch.h"

#include "ComputeShader.h"
#include "Camera.h"
#include "FrameResource.h"

ComputeShader::ComputeShader()
{
}

ComputeShader::~ComputeShader()
{
}

bool ComputeShader::Initialize()
{
	if (!AppBase::Initialize())
	{
		return false;
	}
	// Init cameara
	{
		CREATE_OBJ(m_camera, Camera);
	}

	AppBase::InitCubemap(L"../../Asset/Skybox/HDRI/", L"SkyboxEnvHDR.dds", L"SkyboxDiffuseHDR.dds",
		L"SkyboxSpecularHDR.dds", L"SkyboxBrdf.dds");

	InitLights();

	// particle Ãß°¡
	{
		m_uavBuffer.Create2D(Display::g_screenWidth, Display::g_screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}

	ThrowIfFailed(m_commandList->Close());
	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForGpu();
	AppBase::SetFrameResource(m_opaqueList.size() + 1 + m_lightSpheres.size(), MAX_LIGHTS);

	return true;
}

void ComputeShader::UpdateGui(const float frameRate)
{
}

void ComputeShader::Render()
{
	auto cmdAlloc = m_curFrameResource->m_commandAllocator;

	ThrowIfFailed(cmdAlloc->Reset());
	ThrowIfFailed(m_commandList->Reset(cmdAlloc, nullptr));

	m_commandList->RSSetViewports(1, &Graphics::mainViewport);
	m_commandList->RSSetScissorRects(1, &Graphics::mainSissorRect);
	ID3D12DescriptorHeap* descHeaps[] = { Graphics::s_Texture.Get() };
	m_commandList->SetDescriptorHeaps(1, descHeaps);
	m_commandList->SetPipelineState(Graphics::computePSO);
	m_commandList->SetComputeRootSignature(Graphics::computeRootSignature);
	m_commandList->SetComputeRootDescriptorTable(0, m_uavBuffer.GetUAV());
	m_commandList->Dispatch(uint32_t(Display::g_screenWidth / 32.0f), uint32_t(Display::g_screenHeight / 32.0f), 1);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_uavBuffer.GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));

	m_commandList->CopyResource(Graphics::g_DisplayPlane[m_frameIndex].GetResource(), m_uavBuffer.GetResource());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_uavBuffer.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
}

void ComputeShader::Update(const float dt)
{
	AppBase::Update(dt);
}
