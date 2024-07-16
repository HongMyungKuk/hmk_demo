#include <EngineCore.h>

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
	int m_test = 0;
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

}

void ModelViewer::RenderScene(void)
{
}