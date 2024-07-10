#include "pch.h"

#include "ModelViewer.h"
#include "GeometryGenerator.h"
#include "Model.h"


ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    SAFE_DELETE(m_model);
}

bool ModelViewer::Initialize()
{
    if (!AppBase::Initialize())
    {
        return false;
    }

    return true;
}

void ModelViewer::Update()
{
    AppBase::Update();
}

void ModelViewer::Render()
{
    AppBase::Render();

}
