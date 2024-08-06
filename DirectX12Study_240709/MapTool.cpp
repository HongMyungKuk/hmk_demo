#include "pch.h"

#include "Camera.h"
#include "DebugQuadTree.h"
#include "Frustum.h"
#include "GeometryGenerator.h"
#include "Input.h"
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
    SAFE_DELETE(m_frustum);
    SAFE_DELETE(m_DebugQaudTree);
    SAFE_DELETE(m_quadTree);
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

            MeshData grid = GeometryGenerator::MakeSphere(0.2f, 25, 25);

            obj->Initialize(m_device, m_commandList, {grid});
            obj->GetMaterialConstCPU().albedoFactor = Vector3(0.2f, 0.6f, 0.2f);
            obj->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(0.0f, 5.0f, 0.0f)));
        }
        m_opaqueList.push_back(obj);
    }

    {
        m_quadTree    = new QuadTree;
        MeshData grid = GeometryGenerator::MakeSquareGrid(255, 255, 50.0f, Vector2(1.0f));

        uint8_t *image = nullptr;
        int width      = 0;
        int height     = 0;
        int channel    = 0;
        ReadImage(&image, "../../Asset/heightmap01.bmp", width, height, channel);

        for (auto &v : grid.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateRotationX(XM_PIDIV2));
            v.normal   = Vector3::Transform(v.normal, Matrix::CreateRotationX(XM_PIDIV2));
        }

        float heightScale = 2.0f;

        for (int i = 0; i < grid.indices.size(); i += 3)
        {
            auto i0 = grid.indices[i];
            auto i1 = grid.indices[i + 1];
            auto i2 = grid.indices[i + 2];

            float h0 = float(image[4 * i0]) / 255.0f;
            float h1 = float(image[4 * i1]) / 255.0f;
            float h2 = float(image[4 * i2]) / 255.0f;

            grid.vertices[i0].position = Vector3::Transform(
                grid.vertices[i0].position, Matrix::CreateTranslation(Vector3(0.0f, h0, 0.0f) * heightScale));
            grid.vertices[i1].position = Vector3::Transform(
                grid.vertices[i1].position, Matrix::CreateTranslation(Vector3(0.0f, h1, 0.0f) * heightScale));
            grid.vertices[i2].position = Vector3::Transform(
                grid.vertices[i2].position, Matrix::CreateTranslation(Vector3(0.0f, h2, 0.0f) * heightScale));
        }

        for (int i = 0; i < grid.indices.size(); i += 3)
        {
            auto i0 = grid.indices[i];
            auto i1 = grid.indices[i + 1];
            auto i2 = grid.indices[i + 2];

            auto v0 = grid.vertices[i0].position;
            auto v1 = grid.vertices[i1].position;
            auto v2 = grid.vertices[i2].position;

            Vector3 normal = (v1 - v0).Cross(v2 - v0);
            normal.Normalize();

            grid.vertices[i0].normal += normal;
            grid.vertices[i1].normal += normal;
            grid.vertices[i2].normal += normal;

            grid.vertices[i0].normal.Normalize();
            grid.vertices[i1].normal.Normalize();
            grid.vertices[i2].normal.Normalize();
        }

        m_quadTree->Initialize(nullptr, {grid}, m_device, m_commandList);
    }

    m_frustum = new Frustum;

    m_DebugQaudTree = new DebugQuadTree;
    m_DebugQaudTree->Initialize(m_device, m_commandList, m_quadTree);

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

    m_quadTree->Update();

    //{
    //    auto cameraPosition = m_camera->GetPosition();

    //    m_quadTree->GetHeight(cameraPosition.x, cameraPosition.z, cameraPosition.y);

    //    m_camera->SetPosition(cameraPosition);
    //}

    float height = 0.0f;
    m_quadTree->GetHeight(m_opaqueList[0]->GetPos().x, m_opaqueList[0]->GetPos().z, height);
    m_opaqueList[0]->UpdateWorldMatrix(
        Matrix::CreateTranslation(Vector3(m_opaqueList[0]->GetPos().x, height, m_opaqueList[0]->GetPos().z)));

    if (GameInput::IsPressed(GameInput::kKey_up))
    {

        m_opaqueList[0]->AddVelocity(Vector3(0.0f, 0.0f, 1.0f));
    }
    if (GameInput::IsPressed(GameInput::kKey_right))
    {

        m_opaqueList[0]->AddVelocity(Vector3(1.0f, 0.0f, 0.0f));
    }
    if (GameInput::IsPressed(GameInput::kKey_left))
    {

        m_opaqueList[0]->AddVelocity(Vector3(-1.0f, 0.0f, 0.0f));
    }
    if (GameInput::IsPressed(GameInput::kKey_down))
    {

        m_opaqueList[0]->AddVelocity(Vector3(0.0f, 0.0f, -1.0f));
    }
    if (GameInput::IsPressed(GameInput::kKey_z))
    {

        m_opaqueList[0]->AddVelocity(Vector3(0.0f, 1.0f, 0.0f));
    }
    if (GameInput::IsPressed(GameInput::kKey_x))
    {

        m_opaqueList[0]->AddVelocity(Vector3(0.0f, -1.0f, 0.0f));
    }

    m_opaqueList[0]->Move(dt);

    m_opaqueList[0]->SetVelocity(Vector3(0.0f));

    m_frustum->ConstructFrustum(m_camera->GetFarZ(), m_globalConstsData.view.Transpose(),
                                m_globalConstsData.proj.Transpose());

    m_DebugQaudTree->Update();

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

    m_commandList->SetPipelineState(m_isWireFrame ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO);

    m_quadTree->Render(m_frustum, m_commandList);

    if (GameInput::IsPressed(GameInput::kKey_b))
        m_DebugQaudTree->Render(m_commandList);
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
        // ImGui::Text("The number of triangles is %d in this frame.", m_quadTree->GetNumRenderTriangles());

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
