#include "pch.h"

#include "Camera.h"
#include "GeometryGenerator.h"
#include "MapTool.h"
#include "Model.h"

// https://sketchfab.com/3d-models/gm-bigcity-f80855b6286944459392fc723ed0b50f#download
// https://free3d.com/3d-model/sci-fi-downtown-city-53758.html

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

    AppBase::InitCubemap(L"../../Asset/Skybox/", L"DGarden_diffuseIBL.dds");

    m_basePath = "../../Asset/City/";
    // Create the terrain
    {
        CREATE_MODEL_OBJ(m_terrain);
        {
            auto [model, material] = GeometryGenerator::ReadFromModelFile(m_basePath.c_str(), "gm_bigcity.obj");
            m_terrain->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model, material);
            m_terrain->UpdateWorldMatrix(Matrix::CreateScale(50.0f, 50.0f, 50.0f) *
                                         Matrix::CreateTranslation(0.0f, -2.0f, 0.0f));
            m_terrain->GetMaterialConstCPU().texFlag = false;
        }
    }

    WaitForPreviousFrame();

    // Create the skybox.
    {
        CREATE_MODEL_OBJ(m_skybox);
        {
            auto cube = GeometryGenerator::MakeCube(100.0f, 100.0f, 100.0f);
            m_skybox->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, {cube});
        }
    }

    WaitForPreviousFrame();

    return true;
}

void MapTool::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    m_terrain->GetMaterialConstCPU().texFlag = m_useTexture;
    m_terrain->Update();
    m_skybox->Update();
}

void MapTool::Render()
{
    AppBase::Render();

    m_commandList->SetPipelineState(m_terrain->GetPSO(m_isWireFrame));
    m_terrain->Render(m_commandList);

    m_commandList->SetPipelineState(Graphics::skyboxPSO);
    m_skybox->Render(m_commandList);
}

void MapTool::UpdateGui(const float frameRate)
{
    ImGuiWindowFlags window_flags = 0;
    if (false)
        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (false)
        window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (true)
        window_flags |= ImGuiWindowFlags_MenuBar;
    if (false)
        window_flags |= ImGuiWindowFlags_NoMove;
    if (false)
        window_flags |= ImGuiWindowFlags_NoResize;
    if (false)
        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (false)
        window_flags |= ImGuiWindowFlags_NoNav;
    if (false)
        window_flags |= ImGuiWindowFlags_NoBackground;
    if (false)
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (false)
        window_flags |= ImGuiWindowFlags_UnsavedDocument;

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Gui Demo", nullptr, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // size tuning.
    g_imguiWidth  = float(g_screenWidth) / 4.0f;
    g_imguiHeight = float(g_screenHeight);
    ImGui::SetWindowSize(ImVec2(float(g_imguiWidth), float(g_imguiHeight)));
    ImGui::SetWindowPos(ImVec2(float(g_screenWidth - g_imguiWidth), 0.0f));
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX       = 0;
    viewport.TopLeftY       = 0;
    viewport.MinDepth       = 0.0f;
    viewport.MaxDepth       = 1.0f;
    viewport.Width          = (FLOAT)g_screenWidth - g_imguiWidth;
    viewport.Height         = (FLOAT)g_screenHeight;
    this->SetViewport(viewport);

    ImGui::Text("App average %.3f ms/frame (%.1f FPS)", 1000.0f / frameRate, frameRate);
    auto cameraSpeed = m_camera->GetCameraSpeed();
    ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.001f, 0.01f);
    if (cameraSpeed != m_camera->GetCameraSpeed())
    {
        m_camera->SetCameraSpeed(cameraSpeed);
    }

    // Section
    if (ImGui::CollapsingHeader("Debugging"))
    {
        ImGui::Checkbox("First person view", &m_isFPV);
        ImGui::Checkbox("Draw as normal", &m_drawAsNormal);
        ImGui::Checkbox("Wire frame", &m_isWireFrame);
        ImGui::Checkbox("Use texture", &m_useTexture);
        ImGui::Checkbox("Use MSAA", &m_useMSAA);
        // static float speed = 0.0f;
        // ImGui::SliderFloat("Character front speed", &speed, 0.0001f, 0.001f, "%.7f");
        // m_model->SetSpeed(speed, FRONT);
    }

    ImGui::End();
}
