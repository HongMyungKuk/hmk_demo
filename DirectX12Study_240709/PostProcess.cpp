#include "pch.h"

#include "AppBase.h"
#include "GeometryGenerator.h"
#include "PostProcess.h"

void PostProcess::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<ColorBuffer*> resource, std::vector<ColorBuffer*> target,
	const int width, const int height, const int bloomLevels)
{
	MeshData square = GeometryGenerator::MakeSquare(2.0f, 2.0f);

	D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.vertexBuffer, square.vertices.data(),
		uint32_t(square.vertices.size() * sizeof(Vertex)));
	D3DUtils::CreateDefaultBuffer(Graphics::g_Device, &mesh.indexBuffer, square.indices.data(),
		uint32_t(square.indices.size() * sizeof(MeshData::index_t)));
	mesh.vertexCount = uint32_t(square.vertices.size());
	mesh.indexCount = uint32_t(square.indices.size());
	mesh.stride = sizeof(Vertex);

	m_bloomBuffers.resize(bloomLevels);
	for (int i = 0; i < bloomLevels; i++)
	{
		int div = int(pow(2, i));
		CreateBuffer(width / div, height / div, m_bloomBuffers[i]);
	}

	m_bloomDownFilter.resize(bloomLevels - 1);
	for (int i = 0; i < bloomLevels - 1; i++)
	{
		int div = int(pow(2, i + 1));
		m_bloomDownFilter[i].Initialize(device, cmdList, Graphics::bloomDownPS, width / div, height / div);
		if (i == 0) {
			m_bloomDownFilter[i].SetShaderResource({ resource[0] });
		}
		else {
			m_bloomDownFilter[i].SetShaderResource({ &m_bloomBuffers[i] });
		}
		m_bloomDownFilter[i].SetRenderTargetView({ &m_bloomBuffers[i + 1] });
	}

	m_bloomUpFilter.resize(bloomLevels - 1);
	for (int i = 0; i < bloomLevels - 1; i++) {
		int level = bloomLevels - 2 - i;
		int div = int(pow(2, level));
		m_bloomUpFilter[i].Initialize(device, cmdList, Graphics::bloomUpPS, width / div, height / div);
		m_bloomUpFilter[i].SetShaderResource({ &m_bloomBuffers[level + 1] });
		m_bloomUpFilter[i].SetRenderTargetView({ &m_bloomBuffers[level] });
	}

	// Combine + ToneMapping
	m_combineFilter.Initialize(device, cmdList, Graphics::combinePS, width,
		height, true);
	m_combineFilter.SetShaderResource({ resource[0], &m_bloomBuffers[0] });
	m_combineFilter.m_constData.strength = 0.5f; // Bloom strength
	m_combineFilter.m_constData.option1 = 1.0f;  // Exposure로 사용
	m_combineFilter.m_constData.option2 = 2.2f;  // Gamma로 사용

	m_combineFilter.m_constsBuffer.Upload(0, &m_combineFilter.m_constData);
}

void PostProcess::CreateBuffer(int width, int height, ColorBuffer& bloomBuffer)
{
	bloomBuffer.Create(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
}

void PostProcess::Update()
{
}

void PostProcess::Render(ID3D12GraphicsCommandList* commandList, int frameIdx)
{
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView());
	commandList->IASetIndexBuffer(&mesh.IndexBufferView());

	if (m_combineFilter.m_constData.strength > 0.0f) {
		for (int i = 0; i < m_bloomDownFilter.size(); i++) {
			RenderImageFilter(commandList, m_bloomDownFilter[i]);
		}

		for (int i = 0; i < m_bloomUpFilter.size(); i++) {
			RenderImageFilter(commandList, m_bloomUpFilter[i]);
		}
	}

	m_combineFilter.SetRenderTargetView({ &Graphics::g_DisplayPlane[frameIdx] });
	RenderImageFilter(commandList, m_combineFilter);
}

void PostProcess::RenderImageFilter(ID3D12GraphicsCommandList* commandList, ImageFilter& imageFilter)
{
	imageFilter.Render();
	commandList->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
}
