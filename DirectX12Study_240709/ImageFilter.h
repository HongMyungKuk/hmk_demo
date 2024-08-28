#pragma once

#include "pch.h"
#include "ColorBuffer.h"
#include "ConstantBuffer.h"

class ImageFilter
{
	struct ImageFilterConstData {
		float dx;
		float dy;
		float threshold;
		float strength;
		float option1;       // exposure in CombinePS.hlsl
		float option2;       // gamma in CombinePS.hlsl
		float option3 = 0.0; // blur in CombinePS.hlsl
		float option4;
	};

public:
	ImageFilter()
	{
	}

	~ImageFilter()
	{
		SAFE_RELEASE(m_pso);
	}

	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3DBlob* psBytes, int width, int height, bool back = false)
	{
		isCombine = back;

		ID3DBlob* ps = psBytes;

		m_device = device;
		m_commandList = commandList;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { Graphics::samplingILDesc.data(), UINT(Graphics::samplingILDesc.size()) };
		psoDesc.pRootSignature = Graphics::postProcessRootSignature;
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(Graphics::samplingVS);
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
		psoDesc.RasterizerState = Graphics::solidMSSACW;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = !back ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = {};
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));

		m_constsBuffer.Initialize(m_device, 1);

		m_constData.dx = 1.0f / width;
		m_constData.dy = 1.0f / height;

		m_constsBuffer.Upload(0, &m_constData);
	}

	void Render()
	{
		m_commandList->RSSetViewports(1, &Graphics::mainViewport);
		m_commandList->RSSetScissorRects(1, &Graphics::mainSissorRect);

		m_commandList->SetGraphicsRootSignature(Graphics::postProcessRootSignature);
		m_commandList->SetGraphicsRootConstantBufferView(2, m_constsBuffer.GetResource()->GetGPUVirtualAddress());

		for (auto& r : m_srvBuffer)
		{
			m_commandList->ResourceBarrier(
				1, &CD3DX12_RESOURCE_BARRIER::Transition(r->GetResource(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		m_commandList->SetGraphicsRootDescriptorTable(0, m_srvBuffer[0]->GetSRV());
		if(isCombine)
			m_commandList->SetGraphicsRootDescriptorTable(1, m_srvBuffer[1]->GetSRV());

		if (isCombine)
		{
			m_commandList->ResourceBarrier(
				1, &CD3DX12_RESOURCE_BARRIER::Transition(m_rtvBuffer[0]->GetResource(),
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}

		m_commandList->OMSetRenderTargets(1, &m_rtvBuffer[0]->GetRTV(), false, nullptr);

		m_commandList->SetPipelineState(m_pso);

		for (auto& r : m_srvBuffer)
		{
			m_commandList->ResourceBarrier(
				1, &CD3DX12_RESOURCE_BARRIER::Transition(r->GetResource(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
	}

	void SetShaderResource(std::vector<ColorBuffer*> resource)
	{
		m_srvBuffer.clear();
		for (auto& r : resource)
		{
			m_srvBuffer.push_back(r);
		}
	}

	void SetRenderTargetView(std::vector<ColorBuffer*> resource)
	{
		m_rtvBuffer.clear();
		for (auto& r : resource)
		{
			m_rtvBuffer.push_back(r);
		}
	}

	void ReleaseShaderResource()
	{
		for (auto& r : m_srvBuffer)
		{
			m_commandList->ResourceBarrier(
				1, &CD3DX12_RESOURCE_BARRIER::Transition(r->GetResource(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
	}

private:
	ID3D12Device* m_device = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12PipelineState* m_pso = nullptr;
	std::vector<ColorBuffer*> m_rtvBuffer;
	std::vector<ColorBuffer*> m_srvBuffer;

public:
	ImageFilterConstData m_constData;
	UploadBuffer<ImageFilterConstData> m_constsBuffer;

	bool isCombine = false;
};

