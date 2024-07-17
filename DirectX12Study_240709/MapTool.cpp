#include "pch.h"

#include "GeometryGenerator.h"
#include "MapTool.h"
#include "Model.h"
#include "Camera.h"

MapTool::MapTool()
{
}

MapTool::~MapTool()
{
    SAFE_DELETE(m_terrain);
}

bool MapTool::Initialize()
{
    if (!AppBase::Initialize())
    {
        return false;
    }

    // Init cameara
    {
        CREATE_OBJ(m_camera, Camera);
    }

    m_basePath = "../../Asset/Sponza/";
    // Create the terrain
    {
        CREATE_MODEL_OBJ(m_terrain);
        {
            auto [model, material] = GeometryGenerator::ReadFromModelFile(m_basePath.c_str(), "sponza.fbx");
            m_terrain->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model, material);
            m_terrain->UpdateWorldMatrix(Matrix::CreateScale(10.0f, 10.0f, 10.0f) *
                                         Matrix::CreateTranslation(0.0f, -2.0f, 0.0f));
            m_terrain->GetMaterialConstCPU().texFlag  = false;
        }
    }

    return true;
}

void MapTool::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    m_terrain->Update();
}

void MapTool::Render()
{
    AppBase::Render();

    m_commandList->SetPipelineState(m_terrain->GetPSO(m_isWireFrame));
    m_terrain->Render(m_commandList);
}

void MapTool::UpdateGui(const float frameRate)
{
}
