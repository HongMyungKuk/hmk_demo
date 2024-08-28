#pragma once

#include "ConstantBuffer.h"
#include "Mesh.h"
#include "ColorBuffer.h"
#include "ImageFilter.h"

class PostProcess
{
public:
	~PostProcess()
	{
		SAFE_RELEASE(mesh.vertexBuffer);
		SAFE_RELEASE(mesh.indexBuffer);
	}

	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<ColorBuffer*> resource, std::vector<ColorBuffer*> target,
		const int width, const int height, const int bloomLevels);
	void CreateBuffer(int width, int height, ColorBuffer& bloomBuffer);
	void Update();
	void Render(ID3D12GraphicsCommandList* commandList, int frameIdx);
	void RenderImageFilter(ID3D12GraphicsCommandList* commandList, ImageFilter& imageFilter);

private:
	Mesh mesh = {};
	uint32_t m_indexCount = 0;

	std::vector<ColorBuffer> m_bloomBuffers;
	std::vector<ImageFilter> m_bloomDownFilter;
	std::vector<ImageFilter> m_bloomUpFilter;

public:
	ImageFilter m_combineFilter;
};
