#include "pch.h"

#include "Camera.h"
#include "GeometryGenerator.h"
#include "Model.h"
#include "ModelViewer.h"

ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    SAFE_DELETE(m_model);
    SAFE_DELETE(m_ground);
    SAFE_DELETE(m_box);
}

bool ModelViewer::Initialize()
{
    if (!AppBase::Initialize())
    {
        return false;
    }

    // Init cameara
    {
        CREATE_OBJ(m_camera, Camera);
    }

    // Create the model.
    {
        CREATE_MODEL_OBJ(m_model);
        {
            auto [model, material] = GeometryGenerator::ReadFromModelFile("../../Asset/Model/", "comp_model.fbx");
            m_model->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model, material);
            m_model->GetMaterialConstCPU().ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_model->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        }
    }

    // Create the box.
    {
        CREATE_MODEL_OBJ(m_box);
        {
            MeshData cube              = GeometryGenerator::MakeCube(1.0f, 1.0f, 1.0f);
            cube.albedoTextureFilename = "boxTex.jpeg";
            m_box->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, {cube});
            m_box->GetMaterialConstCPU().diffuse = XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_box->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        }
    }

    WaitForPreviousFrame();

    // Create the ground.
    {
        CREATE_MODEL_OBJ(m_ground);
        {
            MeshData square              = GeometryGenerator::MakeSquare(10.0f, 10.0f);
            square.albedoTextureFilename = "../../Asset/Tiles105_4K-JPG/Tiles105_4K-JPG_Color.jpg";
            m_ground->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, {square});
            m_ground->GetMaterialConstCPU().diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
            m_ground->GetMaterialConstCPU().texFlag = true;
            m_ground->UpdateWorldMatrix(XMMatrixRotationX(XMConvertToRadians(90.0f)));
        }
    }

    WaitForPreviousFrame();

    return true;
}

void ModelViewer::Update()
{
    AppBase::Update();

    static float dt = 0.0f;
    dt              = 1.0f / 10.0f;

    UpdateCamera(dt);

    m_model->GetMaterialConstCPU().texFlag = m_useTexture;
    m_model->Update();
    m_box->Update();
    m_ground->Update();
}

void ModelViewer::Render()
{
    AppBase::Render();

    m_commandList->SetPipelineState(m_model->GetPSO(m_isWireFrame));
    m_model->Render(m_commandList);

    // m_commandList->SetPipelineState(m_box->GetPSO());
    // m_box->Render(m_device, m_commandList);

    m_commandList->SetPipelineState(m_ground->GetPSO(m_isWireFrame));
    m_ground->Render(m_commandList);

    if (m_drawAsNormal) {
        // Set normal PSO
        m_commandList->SetPipelineState(Graphics::normalPSO);
        m_model->RenderNormal(m_commandList);
    }
}

void ModelViewer::UpdateGui()
{
    bool p_open                   = true;
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
    if (false)
        p_open = NULL; // Don't pass our bool* to Begin
    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Gui Demo", &p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            ImGui::EndMenu();
        }
        // if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Section
    if (ImGui::CollapsingHeader("Debugging"))
    {
        ImGui::Checkbox("First person view", &m_isFPV);
        ImGui::Checkbox("Draw as normal", &m_drawAsNormal);
        ImGui::Checkbox("Wire frame", &m_isWireFrame);
        ImGui::Checkbox("Use texture", &m_useTexture);
        ImGui::Checkbox("Use MSAA", &m_useMSAA);
    }
    // Mouse & keyboard
    if (ImGui::CollapsingHeader("Inputs"))
    {
    }

    ImGui::End();
}
