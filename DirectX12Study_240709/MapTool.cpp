#include "pch.h"

#include "Camera.h"
#include "GeometryGenerator.h"
#include "MapTool.h"
#include "Model.h"
#include "QuadTree.h"

// https://sketchfab.com/3d-models/gm-bigcity-f80855b6286944459392fc723ed0b50f#download
// https://free3d.com/3d-model/sci-fi-downtown-city-53758.html

MapTool::MapTool()
{
}

MapTool::~MapTool()
{
    SAFE_DELETE(m_skybox);
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

    AppBase::InitCubemap(L"../../Asset/Skybox/HDRI/", L"SkyboxEnvHDR.dds", L"SkyboxDiffuseHDR.dds",
                         L"SkyboxSpecularHDR.dds", L"SkyboxBrdf.dds");

    // Create the grid.
    {
        Model *obj = nullptr;
        CREATE_MODEL_OBJ(obj);
        {
            const float texCoordSacle = 25.0f;

            MeshData grid              = GeometryGenerator::MakeSquareGrid(1024, 1024, 50.0f, Vector2(texCoordSacle));
            grid.albedoTextureFilename = "../../Asset/GroundDirtRocky020_COL_4K.jpg";
            grid.heightTextureFilename = "../../Asset/heightmap01.bmp";
            obj->Initialize(m_device, m_commandList, {grid});
            obj->GetMaterialConstCPU().albedoFactor = Vector3(0.8f);
            obj->GetMaterialConstCPU().useAlbedoMap = true;
            obj->GetMeshConstCPU().useHeightMap     = true;
            obj->GetMeshConstCPU().texCoordScale    = texCoordSacle;
            obj->UpdateWorldMatrix(Matrix::CreateRotationX(XM_PIDIV2));
        }
        m_opaqueList.push_back(obj);
    }

    {
        QuadTree *quadTree = new QuadTree;
        MeshData grid      = GeometryGenerator::MakeSquareGrid(1024, 1024, 50.0f, Vector2(1.0f));



        quadTree->Initialize(nullptr, {grid});
    }

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForPreviousFrame();

    return true;
}

void MapTool::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    m_postProcess.GetConstCPU().exposure     = m_exposureFactor;
    m_postProcess.GetConstCPU().gammeaFactor = m_gammaFactor;
    m_postProcess.Update();
}

void MapTool::UpdateLights()
{
    AppBase::UpdateLights();

    if (m_useDL)
    {
        m_light[0].type |= DIRECTIONAL_LIGHT;
    }
    else
    {
        m_light[0].type &= LIGHT_OFF;
    }
    if (m_usePL)
    {
        m_light[1].type |= POINT_LIGHT;
        m_light[1].type |= SHADOW_MAP;
    }
    else
    {
        m_light[1].type &= LIGHT_OFF;
    }
    if (m_useSL)
    {
        m_light[2].type |= SPOT_LIGHT;
        m_light[2].type |= SHADOW_MAP;
    }
    else
    {
        m_light[2].type &= LIGHT_OFF;
    }

    m_globalConstsData.lights[0] = m_light[0];
    m_globalConstsData.lights[1] = m_light[1];
    m_globalConstsData.lights[2] = m_light[2];

    for (uint32_t i = 0; i < 3; i++)
    {
        if (m_light[i].type & POINT_LIGHT)
        {
            m_lightSpheres[0]->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(m_light[i].position)));
        }
        if (m_light[i].type & SPOT_LIGHT)
        {
            m_lightSpheres[1]->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(m_light[i].position)));
        }
    }

    for (const auto &l : m_lightSpheres)
    {
        l->Update();
    }
}

void MapTool::Render()
{
    AppBase::Render();

    m_commandList->RSSetViewports(1, &Graphics::mainViewport);
    m_commandList->RSSetScissorRects(1, &Graphics::mainSissorRect);

    // Root signature 이후에 변경 .... 방법 찾기
    m_commandList->SetGraphicsRootSignature(Graphics::defaultRootSignature);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_globalConstsBuffer.GetResource()->GetGPUVirtualAddress());

    m_commandList->SetGraphicsRootDescriptorTable(3, Graphics::s_Texture[1]);

    for (uint32_t i = 0; i < 3; i++)
    {
        if (m_light[i].type & POINT_LIGHT)
        {
            m_commandList->SetPipelineState(m_lightSpheres[0]->GetPSO(m_isWireFrame));
            m_lightSpheres[0]->Render(m_commandList);
        }
        if (m_light[i].type & SPOT_LIGHT)
        {
            m_commandList->SetPipelineState(m_lightSpheres[1]->GetPSO(m_isWireFrame));
            m_lightSpheres[1]->Render(m_commandList);
        }
    }
}

void MapTool::UpdateGui(const float frameRate)
{
    using namespace Display;

    AppBase::UpdateGui(frameRate);

    // size tuning.
    // g_imguiWidth  = float(g_screenWidth) / 4.0f;
    // g_imguiHeight = float(g_screenHeight);
    // ImGui::SetWindowSize(ImVec2(float(g_imguiWidth), float(g_imguiHeight)));
    // ImGui::SetWindowPos(ImVec2(float(g_screenWidth - g_imguiWidth), 0.0f));
    // Graphics::mainViewport =
    //    D3DUtils::CreateViewport(0.0f, 0.0f, (float)(g_screenWidth - g_imguiWidth), (float)g_screenHeight);
    // Graphics::mainSissorRect = D3DUtils::CreateScissorRect(0, 0, long(g_screenWidth - g_imguiWidth), g_screenHeight);

    // Section
    if (ImGui::CollapsingHeader("Debugging"))
    {
        ImGui::Text("The number of triangles is %d in this frame.", m_opaqueList[0]->GetNumRenderTriangles());

        ImGui::Checkbox("First person view", &m_isFPV);
        ImGui::Checkbox("Draw as normal", &m_drawAsNormal);
        ImGui::Checkbox("Wire frame", &m_isWireFrame);
        ImGui::Checkbox("Use texture", &m_useTexture);
        ImGui::Checkbox("Use MSAA", &m_useMSAA);
        // static float speed = 0.0f;
        // ImGui::SliderFloat("Character front speed", &speed, 0.0001f, 0.001f, "%.7f");
        // m_model->SetSpeed(speed, FRONT);
    }

    // Light
    if (ImGui::CollapsingHeader("Lights"))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Directional Light"))
        {
            ImGui::Checkbox("Use directional light", &m_useDL);
            ImGui::Text("Current light direction: (%.0f, %.0f, %.0f)", m_light[0].direction.x, m_light[0].direction.y,
                        m_light[0].direction.z);
            ImGui::SliderFloat3("light direction", &m_light[0].direction.x, -5.0f, 5.0f);
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNode("Point Light"))
        {
            ImGui::Checkbox("Use point light", &m_usePL);
            ImGui::SliderFloat3("light position", &m_light[1].position.x, -5.0f, 5.0f);
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNode("Spot Light"))
        {
            ImGui::Checkbox("Use spot light", &m_useSL);
            ImGui::SliderFloat("Spot power", &m_light[2].spotPower, 1.0f, 32.0f);
            ImGui::SliderFloat3("light direction", &m_light[2].direction.x, -5.0f, 5.0f);
            ImGui::SliderFloat3("light position", &m_light[2].position.x, -5.0f, 5.0f);
            ImGui::TreePop();
            ImGui::Spacing();
        }
    }

    ImGui::End();
}
