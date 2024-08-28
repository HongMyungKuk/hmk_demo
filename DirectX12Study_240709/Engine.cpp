#include "pch.h"

#include "BillboardModel.h"
#include "Camera.h"
#include "Command.h"
#include "DebugQuadTree.h"
#include "Engine.h"
#include "Frustum.h"
#include "GeometryGenerator.h"
#include "Input.h"
#include "Model.h"
// #include "QuadTree.h"
#include "OceanModel.h"
#include "Terrain.h"
// https://sketchfab.com/3d-models/gm-bigcity-f80855b6286944459392fc723ed0b50f#download
// https://free3d.com/3d-model/sci-fi-downtown-city-53758.html

Engine::Engine()
{
}

Engine::~Engine()
{
    SAFE_DELETE(m_frustum);
    SAFE_DELETE(m_DebugQaudTree);
    SAFE_DELETE(m_terrain);
    SAFE_DELETE(m_skybox);
    SAFE_DELETE(m_terrain);
    SAFE_RELEASE(m_uploadResource);
    SAFE_RELEASE(m_terrainTexResource);
}

bool Engine::Initialize()
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

    //// Create the grid.
    //{
    //    Model *obj = nullptr;
    //    CREATE_MODEL_OBJ(obj);
    //    {
    //        const float texCoordSacle = 25.0f;
    //        float radius              = 0.2f;
    //        MeshData sphere           = GeometryGenerator::MakeSphere(radius, 25, 25);

    //        obj->Initialize(m_device, m_commandList, {sphere});
    //        obj->GetMaterialConstCPU().albedoFactor = Vector3(0.2f, 0.6f, 0.2f);
    //        obj->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(0.0f, 5.0f, 0.0f)));
    //    }
    //    m_opaqueList.push_back(obj);
    //}

    {
        m_terrain        = new Terrain;
        MeshData grid    = GeometryGenerator::MakeSquareGrid(255, 255, 50.0f, Vector2(25.0f));
        s_TerrainSRV     = Graphics::s_Texture.Alloc(1);
        m_uploadResource = D3DUtils::CreateTexture(m_device, m_commandList, "../../Asset/GroundDirtRocky020_COL_4K.jpg",
                                                   &m_terrainTexResource, D3D12_CPU_DESCRIPTOR_HANDLE(s_TerrainSRV));

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

        float heightScale = 0.5f;

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

        m_terrain->Initialize(m_device, m_commandList, {grid});
    }

    std::cout << m_terrain->GetMeshComponentSize() << std::endl;

    m_frustum = new Frustum;

    m_DebugQaudTree = new DebugQuadTree;
    m_DebugQaudTree->Initialize(m_device, m_commandList, m_terrain);

    // Create the model.
    {
        Model *skinnedModel = new SkinnedMeshModel;
        if (!skinnedModel)
        {
            return false;
        }
        {
            std::string basePath               = "../../Asset/Model/";
            std::vector<std::string> animClips = {"idle.fbx", "Running_60.fbx", "Right Strafe Walking.fbx",
                                                  "Left Strafe Walking.fbx", "Walking Backward.fbx"};

            AnimationData animData = {};
            for (const auto &clip : animClips)
            {
                auto [_, anim] = GeometryGenerator::ReadFromAnimationFile(basePath.c_str(), clip.c_str());

                if (animData.clips.empty())
                {
                    animData = anim;
                }
                else
                {
                    animData.clips.push_back(anim.clips.front());
                }
            }

            auto [model, material] = GeometryGenerator::ReadFromModelFile(basePath.c_str(), "comp_model.fbx", true);

            ((SkinnedMeshModel *)skinnedModel)->Initialize(m_device, m_commandList, model, material, animData);
            skinnedModel->GetMaterialConstCPU().useAlbedoMap = m_useTexture;
            skinnedModel->GetMaterialConstCPU().albedoFactor = Vector3(0.3f);
            // skinnedModel->UpdateWorldMatrix(Matrix::CreateRotationY(XM_PI));
        }
        m_opaqueList.push_back(skinnedModel);
    }

    // m_temple        = new Model;
    // auto [model, _] = GeometryGenerator::ReadFromModelFile("../../Asset/", "temple_test.fbx");
    // m_temple->Initialize(m_device, m_commandList, model, {}, false);
    // m_temple->UpdateWorldMatrix(Matrix::CreateScale(25.0f));
    // m_opaqueList.push_back(m_temple);

    // 태양 구현
    m_billBoardSun = new BillboardModel;
    ((BillboardModel *)m_billBoardSun)->Initialize(m_device, m_commandList, {Vector4(30.0f, 30.0f, 30.0f, 1.0f)}, 1.0f);
    m_opaqueList.push_back(m_billBoardSun);

    // 바다
    {
        auto mesh        = GeometryGenerator::MakeSquare(200.0, 200.0f);
        m_ocean          = new OceanModel(m_device, m_commandList, std::vector{mesh});
        Vector3 position = Vector3(0.0f, -0.1f, 2.0f);
        m_ocean->UpdateWorldMatrix(Matrix::CreateRotationX(3.141592f * 0.5f) * Matrix::CreateTranslation(position));
        m_ocean->m_castShadow = false;
        m_opaqueList.push_back(m_ocean);
    }

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForGpu();

    m_useDL = false;
    m_usePL = true;
    m_useSL = true;

    //// Set event handler.
    // g_EvnetHandler.RegistObjectMoveCommand(EventHandler::COMMAND_TYPE::OBJ,
    //                                        new ObjectMoveFrontCommand(m_opaqueList[0], m_light, m_camera));

    // global const setting.
    m_globalConstsData.envStrength = 0.0f;

    return true;
}

void Engine::Update(const float dt)
{
    AppBase::Update(dt);

    // UpdateCamera(dt);

    m_opaqueList[0]->SetVelocity(Vector3(0.0f));
    m_opaqueList[0]->SetSpeed(2.5f);

    Vector3 translation = m_opaqueList[0]->GetWorldRow().Translation();
    m_opaqueList[0]->GetWorldRow().Translation(Vector3(0.0f));
    m_opaqueList[0]->UpdateWorldMatrix(Matrix::CreateRotationY(XM_PI) * Matrix::CreateRotationY(m_camera->GetPitch()) *
                                       Matrix::CreateTranslation(translation));

    m_terrain->Update();

    // 캐릭터가 튀는 현상이 발생.
    float height            = 0.0f; // object radius.
    static float prevHeight = 0.0f;
    // static bool heightFlag  = false;
    // if (heightFlag)
    //{
    //     m_camera->SetPosition(m_camera->GetPosition() + Vector3(0.0f, -prevHeight, 0.0f));
    //     m_light[1].position =
    //         Vector3::Transform(m_light[1].position, Matrix::CreateTranslation(Vector3(0.0f, -prevHeight, 0.0f)));
    //     heightFlag = false;
    // }

    m_camera->SetPosition(m_camera->GetPosition() + Vector3(0.0f, -prevHeight, 0.0f));
    m_light[1].position =
        Vector3::Transform(m_light[1].position, Matrix::CreateTranslation(Vector3(0.0f, -prevHeight, 0.0f)));

    m_terrain->GetObjectHeight(m_opaqueList[0]->GetPos().x, m_opaqueList[0]->GetPos().z, &height);

    // if (height != prevHeight)
    //{
    //     translation = m_opaqueList[0]->GetWorldRow().Translation();
    //     m_opaqueList[0]->GetWorldRow().Translation(Vector3(0.0f));
    //     m_opaqueList[0]->UpdateWorldMatrix(
    //         m_opaqueList[0]->GetWorldRow() *
    //         Matrix::CreateTranslation(Vector3(m_opaqueList[0]->GetPos().x, height, m_opaqueList[0]->GetPos().z)));

    //    m_camera->SetPosition(m_camera->GetPosition() + Vector3(0.0f, height, 0.0f));
    //    m_light[1].position =
    //        Vector3::Transform(m_light[1].position, Matrix::CreateTranslation(Vector3(0.0f, height, 0.0f)));
    //    // heightFlag = true;
    //}

    // prevHeight = height;

    translation = m_opaqueList[0]->GetWorldRow().Translation();
    m_opaqueList[0]->GetWorldRow().Translation(Vector3(0.0f));
    m_opaqueList[0]->UpdateWorldMatrix(
        m_opaqueList[0]->GetWorldRow() *
        Matrix::CreateTranslation(Vector3(m_opaqueList[0]->GetPos().x, height, m_opaqueList[0]->GetPos().z)));

    prevHeight = height;

    m_camera->SetPosition(m_camera->GetPosition() + Vector3(0.0f, height, 0.0f));
    m_light[1].position =
        Vector3::Transform(m_light[1].position, Matrix::CreateTranslation(Vector3(0.0f, height, 0.0f)));

    if (((SkinnedMeshModel *)m_opaqueList[0])->GetAnim().clips.size() > 0)
    {
        // update animation.

        {
            static int frameCount = 0;
            static int state      = 0;

            if (GameInput::IsFirstPressed(GameInput::kKey_w))
            {
                if (state == 0)
                {
                    state = 1;
                }
            }
            if (GameInput::IsFirstPressed(GameInput::kKey_d))
            {
                if (state == 0)
                {
                    state = 2;
                }
            }
            if (GameInput::IsFirstPressed(GameInput::kKey_a))
            {
                if (state == 0)
                {
                    state = 3;
                }
            }
            if (GameInput::IsFirstPressed(GameInput::kKey_s))
            {
                if (state == 0)
                {
                    state = 4;
                }
            }
            if (GameInput::IsFirstReleased(GameInput::kKey_w) || GameInput::IsFirstReleased(GameInput::kKey_d) ||
                GameInput::IsFirstReleased(GameInput::kKey_a) || GameInput::IsFirstReleased(GameInput::kKey_s))
            {
                state = 0;
            }

            // TODO!!
            // 애니메시연 Event handler 통합하기

            // m_opaqueList[0]->SetSpeed(0.005f);

            if (GameInput::IsPressed(GameInput::kKey_w))
            {
                state = 1;
                m_opaqueList[0]->AddVelocity(m_camera->GetDirection());
                m_camera->SetCameraSpeed(m_opaqueList[0]->GetSpeed());
                m_camera->MoveFront(dt);
                m_light[1].position = Vector3::Transform(
                    m_light[1].position,
                    Matrix::CreateTranslation(m_opaqueList[0]->GetSpeed() * m_opaqueList[0]->GetVelocity() * dt));
            }
            else if (GameInput::IsPressed(GameInput::kKey_d))
            {
                state = 2;
                m_opaqueList[0]->AddVelocity(m_camera->GetRightDirection());
                m_camera->SetCameraSpeed(m_opaqueList[0]->GetSpeed());
                m_camera->MoveRight(dt);
                m_light[1].position = Vector3::Transform(
                    m_light[1].position,
                    Matrix::CreateTranslation(m_opaqueList[0]->GetSpeed() * m_opaqueList[0]->GetVelocity() * dt));
            }
            else if (GameInput::IsPressed(GameInput::kKey_a))
            {
                state = 3;
                m_opaqueList[0]->AddVelocity(-m_camera->GetRightDirection());
                m_camera->SetCameraSpeed(m_opaqueList[0]->GetSpeed());
                m_camera->MoveLeft(dt);
                m_light[1].position = Vector3::Transform(
                    m_light[1].position,
                    Matrix::CreateTranslation(m_opaqueList[0]->GetSpeed() * m_opaqueList[0]->GetVelocity() * dt));
            }
            else if (GameInput::IsPressed(GameInput::kKey_s))
            {
                state = 4;
                m_opaqueList[0]->AddVelocity(-m_camera->GetDirection());
                m_camera->SetCameraSpeed(m_opaqueList[0]->GetSpeed());
                m_camera->MoveBack(dt);
                m_light[1].position = Vector3::Transform(
                    m_light[1].position,
                    Matrix::CreateTranslation(m_opaqueList[0]->GetSpeed() * m_opaqueList[0]->GetVelocity() * dt));
            }

            // if (m_aniPlayFlag)
            //     state = m_selectedAnim;

            m_opaqueList[0]->Move(dt);

            ((SkinnedMeshModel *)m_opaqueList[0])->UpdateAnimation(state, frameCount++);
        }
    }

    m_frustum->ConstructFrustum(m_camera->GetFarZ(), m_globalConstsData.view.Transpose(),
                                m_globalConstsData.proj.Transpose());

    m_DebugQaudTree->Update();

    m_postProcess.GetConstCPU().exposure     = m_exposureFactor;
    m_postProcess.GetConstCPU().gammeaFactor = m_gammaFactor;

    m_postProcess.Update();
}

void Engine::UpdateLights()
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

    if (GameInput::IsFirstPressed(GameInput::kKey_b))
    {
        m_isDebugTreeFlag = !m_isDebugTreeFlag;
    }
}

void Engine::Render()
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
    m_terrain->Render(m_frustum);

    if (m_isDebugTreeFlag)
    {
        m_commandList->SetPipelineState(Graphics::defaultWirePSO);
        m_DebugQaudTree->Render(m_commandList);
    }
}

void Engine::UpdateGui(const float frameRate)
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

    // Ocean
    if (ImGui::CollapsingHeader("Ocean"))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        ImGui::SliderFloat("height", &m_height, 0.0f, 1.0f);
        m_ocean->UpdateWorldMatrix(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                   Matrix::CreateTranslation(Vector3(0.0f, m_height, 2.0f)));
    }

    ImGui::End();
}
