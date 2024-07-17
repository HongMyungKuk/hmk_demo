#include "EngineCore.h"
#include "CommandContext.h"

#pragma comment(lib, "EngineCore.lib")

class ModelViewer : public EngineCore::EngineApp
{
public:
	ModelViewer() {}

	virtual void Startup(void)  override;
	virtual void Cleanup(void)  override;

	virtual void Update(float dt)  override;
	virtual void RenderScene(void)  override;

private:
	D3D12_VIEWPORT m_mainViewport;
	D3D12_RECT m_mainSicssor;
};

CREATE_APPLICATION(ModelViewer);

void ModelViewer::Startup(void)
{

}
void ModelViewer::Cleanup(void)
{

}

void ModelViewer::Update(float dt)
{
	m_mainViewport.Width = (float)1280;
	m_mainViewport.Height = (float)800;
	m_mainViewport.MinDepth = 0.0f;
	m_mainViewport.MaxDepth = 1.0f;

	m_mainSicssor.left = 0;
	m_mainSicssor.top = 0;
	m_mainSicssor.right = (LONG)1280;
	m_mainSicssor.bottom = (LONG)800;
}

void ModelViewer::RenderScene(void)
{
	// set command list & command allocator.
	GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

	const D3D12_VIEWPORT& viewport = m_mainViewport;
	const D3D12_RECT& scissor = m_mainSicssor;



}