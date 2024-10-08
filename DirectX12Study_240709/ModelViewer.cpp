#include "pch.h"

#include "Camera.h"
#include "Command.h"
#include "GeometryGenerator.h"
#include "Input.h"
#include "Model.h"
#include "ModelViewer.h"
#include "SkinnedMeshModel.h"
#include "FrameResource.h"

ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    SAFE_DELETE(m_coordController);
    // SAFE_DELETE(m_model);
    SAFE_DELETE(m_ground);
    SAFE_DELETE(m_sphere);
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

    AppBase::InitCubemap(L"../../Asset/Skybox/HDRI/", L"SkyboxEnvHDR.dds", L"SkyboxDiffuseHDR.dds",
                         L"SkyboxSpecularHDR.dds", L"SkyboxBrdf.dds");

    //// Create the coordinate controller.
    //{
    //    CREATE_MODEL_OBJ(m_coordController);
    //    {
    //        const float radius = 0.3f;

    //        MeshData sphere              = GeometryGenerator::MakeSphere(radius, 25, 25);
    //        sphere.albedoTextureFilename = "../../Asset/CoordTexture.png";
    //        m_coordController->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, {sphere});
    //        m_coordController->GetMaterialConstCPU().texFlag = true;
    //        m_coordController->UpdateWorldMatrix(Matrix::CreateTranslation(m_controllerCenter));

    //        // bounding sphere.
    //        m_boundingSphere = BoundingSphere(Vector3(m_controllerCenter), radius);
    //    }
    //}

    // Create the model.
    {
        Model *skinnedModel = new SkinnedMeshModel;
        if (!skinnedModel)
        {
            return false;
        }
        {
            m_basPath   = "../../Asset/Model/";
            m_animClips = {"idle.fbx", "Running_60.fbx", "Right Strafe Walking.fbx", "Left Strafe Walking.fbx",
                           "Walking Backward.fbx"};

            AnimationData animData = {};
            for (const auto &clip : m_animClips)
            {
                auto [_, anim] = GeometryGenerator::ReadFromAnimationFile(m_basPath.c_str(), clip.c_str());

                if (animData.clips.empty())
                {
                    animData = anim;
                }
                else
                {
                    animData.clips.push_back(anim.clips.front());
                }
            }

            auto [model, material] = GeometryGenerator::ReadFromModelFile(m_basPath.c_str(), "comp_model.fbx");

            ((SkinnedMeshModel *)skinnedModel)->Initialize(m_device, m_commandList, model, material, animData);
            skinnedModel->GetMaterialConstCPU().useAlbedoMap = m_useTexture;
            skinnedModel->GetMaterialConstCPU().albedoFactor = Vector3(0.3f);
            skinnedModel->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        }
        m_opaqueList.push_back(skinnedModel);
    }

    //// Create the grid.
    //{
    //    Model *obj = nullptr;
    //    CREATE_MODEL_OBJ(obj);
    //    {
    //        MeshData grid                 = GeometryGenerator::MakeSquareGrid(256, 256, 1.0f, Vector2(1.0f));
    //        grid.albedoTextureFilename    = "../../Asset/brick-wall-ue/brick-wall_albedo.png";
    //        grid.normalTextureFilename    = "../../Asset/brick-wall-ue/brick-wall_normal-dx.png";
    //        grid.heightTextureFilename    = "../../Asset/brick-wall-ue/brick-wall_height.png";
    //        grid.metallicTextureFilename  = "../../Asset/brick-wall-ue/brick-wall_metallic.png";
    //        grid.roughnessTextureFilename = "../../Asset/brick-wall-ue/brick-wall_roughness.png";
    //        grid.aoTextureFilename        = "../../Asset/brick-wall-ue/brick-wall_ao.png";
    //        obj->Initialize(m_device, m_commandList, {grid});
    //        obj->GetMaterialConstCPU().albedoFactor    = Vector3(0.8f);
    //        obj->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(0.0f, 0.5f, -2.0f)));
    //    }
    //    m_opaqueList.push_back(obj);
    //}

    //// Create the sphere.
    //{
    //    Model *obj = nullptr;
    //    CREATE_MODEL_OBJ(obj);
    //    {
    //        MeshData cube              = GeometryGenerator::MakeCube(1.0f, 1.0f, 1.0f);
    //        cube.albedoTextureFilename = "../../Asset/brick-wall-ue/brick-wall_albedo.png";
    //        cube.normalTextureFilename = "../../Asset/brick-wall-ue/brick-wall_normal-dx.png";
    //        cube.heightTextureFilename = "../../Asset/brick-wall-ue/brick-wall_height.png";
    //        obj->Initialize(m_device, m_commandList, {cube});
    //        obj->GetMaterialConstCPU().albedoFactor    = Vector3(0.8f);
    //        obj->GetMaterialConstCPU().useAlbedoMap    = true;
    //        obj->GetMaterialConstCPU().useMetalnessMap = false;
    //        obj->GetMaterialConstCPU().useNormalMap    = true;
    //        obj->GetMeshConstCPU().useHeightMap        = true;
    //        obj->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(1.5f, 0.5f, 0.0f)));
    //    }
    //    m_opaqueList.push_back(obj);
    //}

    //// Create the sphere.
    //{
    //    Model *obj = nullptr;
    //    CREATE_MODEL_OBJ(obj);
    //    {
    //        MeshData sphere = GeometryGenerator::MakeSphere(0.6f, 25, 25);
    //        obj->Initialize(m_device, m_commandList, {sphere});
    //        obj->GetMaterialConstCPU().albedoFactor    = Vector3(1.0f, 1.0f, 1.0f);
    //        obj->GetMaterialConstCPU().useAlbedoMap    = false;
    //        obj->GetMaterialConstCPU().useMetalnessMap = false;
    //        obj->GetMaterialConstCPU().useNormalMap    = false;

    //        obj->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(-0.5f, 1.0f, 0.0f)));
    //    }
    //    m_opaqueList.push_back(obj);
    //}

    //// Create the square.
    //{
    //    Model *squareObj = nullptr;
    //    CREATE_MODEL_OBJ(squareObj);
    //    {
    //        MeshData square              = GeometryGenerator::MakeSquare(10.0f, 10.0f);
    //        square.albedoTextureFilename = "../../Asset/Bricks075A_4K-JPG/Bricks075A_4K-JPG_Color.jpg";
    //        square.normalTextureFilename = "../../Asset/Bricks075A_4K-JPG/Bricks075A_4K-JPG_NormalDX.jpg";
    //        squareObj->Initialize(m_device, m_commandList, {square});
    //        squareObj->GetMaterialConstCPU().useAlbedoMap = true;
    //        squareObj->GetMaterialConstCPU().useNormalMap = true;
    //        squareObj->GetMaterialConstCPU().albedoFactor = Vector3(1.0f);
    //    }
    //    m_opaqueList.push_back(squareObj);
    //}

    // Create the ground.
    {
        Model *ground = nullptr;
        CREATE_MODEL_OBJ(ground);
        {
            MeshData square = GeometryGenerator::MakeSquare(10.0f, 10.0f);
            square.albedoTextureFilename = "../../Asset/Tiles105_4K-JPG/Tiles105_4K-JPG_Color.jpg";
            ground->Initialize(m_device, m_commandList, {square});
            ground->GetMaterialConstCPU().albedoFactor    = Vector3(0.3f);
            ground->GetMaterialConstCPU().useAlbedoMap    = true;
            ground->GetMaterialConstCPU().useMetalnessMap = false;
            ground->UpdateWorldMatrix(XMMatrixRotationX(XMConvertToRadians(90.0f)));
        }
        m_opaqueList.push_back(ground);
    }

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForGpu();

    //// Set event handler.
    //g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::FRONT,
    //                                       new ObjectMoveFrontCommand(m_opaqueList[0], m_light));
    //g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::BACK,
    //                                       new ObjectMoveBackCommand(m_opaqueList[0], m_light));
    //g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::RIGHT,
    //                                       new ObjectMoveRightCommand(m_opaqueList[0], m_light));
    //g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::LEFT,
    //                                       new ObjectMoveLeftCommand(m_opaqueList[0], m_light));

    // global const setting.
    m_globalConstsData.envStrength = 0.5f;
    

    return true;
}

void ModelViewer::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    // ChangeModel();

    //{
    //    m_coordController->GetMaterialConstCPU().diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
    //    m_coordController->Update();
    //}

    //// picking
    //{
    //    if (m_leftButtonDown)
    //    {
    //        Vector4 ndcNear = Vector4(m_ndcX, m_ndcY, 0.0f, 1.0f);
    //        Vector4 ndcFar  = Vector4(m_ndcX, m_ndcY, 1.0f, 1.0f);

    //        auto viewRow = m_camera->GetViewMatrix();
    //        auto projRow = m_camera->GetProjectionMatrix();

    //        auto InverseViewProjection = (viewRow * projRow).Invert();

    //        auto worldNear = Vector4::Transform(ndcNear, InverseViewProjection);
    //        auto worldFar  = Vector4::Transform(ndcFar, InverseViewProjection);
    //        worldNear /= worldNear.w;
    //        worldFar /= worldFar.w;

    //        Vector3 worldNear3 = Vector3(worldNear);
    //        Vector3 worldFar3  = Vector3(worldFar);

    //        Vector3 dir = worldFar3 - worldNear3;
    //        dir.Normalize();

    //        SimpleMath::Ray curRay = SimpleMath::Ray(worldNear3, dir);
    //        float dist             = 0.0f;
    //        bool isPicking         = false;

    //        // 화면이 Imgui 크기 만큼 차지하고 있어 마우스의 ndc scale이 변경되었음.
    //        // 현재 수정 완료.
    //        m_boundingSphere.Center = Vector3::Transform(m_controllerCenter, viewRow.Invert());
    //        isPicking               = curRay.Intersects(m_boundingSphere, dist);
    //        if (isPicking)
    //        {
    //            Vector3 collesionPositon = worldNear3 + dist * dir;

    //            SimpleMath::Quaternion q;
    //            static Vector3 prevVector(0.0f);
    //            if (m_leftButtonDragStart)
    //            {
    //                m_leftButtonDragStart = false;
    //                prevVector            = collesionPositon - m_boundingSphere.Center;
    //                prevVector.Normalize();
    //            }
    //            else
    //            {
    //                Vector3 currentVector = collesionPositon - m_boundingSphere.Center;
    //                currentVector.Normalize();

    //                if ((currentVector - prevVector).Length() > 1e-5)
    //                {
    //                    q          = SimpleMath::Quaternion::FromToRotation(prevVector, currentVector);
    //                    prevVector = currentVector;
    //                }
    //            }

    //            auto translation = m_coordController->GetWorldRow().Translation();
    //            m_coordController->GetWorldRow().Translation(Vector3(0.0f));
    //            m_coordController->UpdateWorldMatrix(m_coordController->GetWorldRow() *
    //                                                 Matrix::CreateFromQuaternion(q) *
    //                                                 Matrix::CreateTranslation(translation));

    //            translation = m_model->GetWorldRow().Translation();
    //            m_model->GetWorldRow().Translation(Vector3(0.0f));
    //            m_model->UpdateWorldMatrix(m_model->GetWorldRow() * Matrix::CreateFromQuaternion(q) *
    //                                       Matrix::CreateTranslation(translation));
    //        }
    //    }
    //}

    // m_model->Update();

    SkinnedMeshModel *model = dynamic_cast<SkinnedMeshModel *>(m_opaqueList[0]);

    if (model != nullptr)
    {
        if (((SkinnedMeshModel *)m_opaqueList[0])->GetAnim().clips.size() > 0)
        {
            // update animation.

            {
                static int frameCount = 0;
                static int state      = 0;

                if (GameInput::IsFirstPressed(GameInput::kKey_up))
                {
                    if (state == 0)
                    {
                        state = 1;
                    }
                }
                if (GameInput::IsFirstPressed(GameInput::kKey_right))
                {
                    if (state == 0)
                    {
                        state = 2;
                    }
                }
                if (GameInput::IsFirstPressed(GameInput::kKey_left))
                {
                    if (state == 0)
                    {
                        state = 3;
                    }
                }
                if (GameInput::IsFirstPressed(GameInput::kKey_down))
                {
                    if (state == 0)
                    {
                        state = 4;
                    }
                }

                // TODO!!
                // 애니메시연 Event handler 통합하기

                if (GameInput::IsPressed(GameInput::kKey_up))
                {
                    state = 1;
                    //g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::FRONT, dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_right))
                {
                    state = 2;
                    //g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::RIGHT, dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_left))
                {
                    state = 3;
                    //g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::LEFT, dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_down))
                {
                    state = 4;
                    //g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::BACK, dt);
                }
                else
                {
                    state = 0;
                }

                if (m_aniPlayFlag)
                    state = m_selectedAnim;

                ((SkinnedMeshModel *)m_opaqueList[0])->UpdateAnimation(state, frameCount++);
            }
        }
    }

    //m_postProcess.GetConstCPU().exposure     = m_exposureFactor;
    //m_postProcess.GetConstCPU().gammeaFactor = m_gammaFactor;
    m_postProcess.Update();
}

void ModelViewer::Render()
{
    AppBase::Render();

    m_commandList->RSSetViewports(1, &Graphics::mainViewport);
    m_commandList->RSSetScissorRects(1, &Graphics::mainSissorRect);

    // Root signature 이후에 변경 .... 방법 찾기
    m_commandList->SetGraphicsRootSignature(Graphics::defaultRootSignature);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_curFrameResource->m_globalConstsBuffer->GetResource()->GetGPUVirtualAddress());

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

void ModelViewer::UpdateGui(const float frameRate)
{
    using namespace Display;

    AppBase::UpdateGui(frameRate);

    //// size tuning.
    // g_imguiWidth  = float(g_screenWidth) / 4.0f;
    // g_imguiHeight = float(g_screenHeight);
    // ImGui::SetWindowSize(ImVec2(float(g_imguiWidth), float(g_imguiHeight)));
    // ImGui::SetWindowPos(ImVec2(float(g_screenWidth - g_imguiWidth), 0.0f));

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ImGui::MenuItem("(File menu)", NULL, false, false);
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                char lpstrFile[100] = "";

                OPENFILENAMEA ofn = {};
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize     = sizeof(ofn);
                ofn.hwndOwner       = m_hwnd;
                ofn.lpstrFilter     = "모든 파일\0*.*";
                ofn.lpstrFile       = lpstrFile;
                ofn.nMaxFile        = 100;
                ofn.lpstrInitialDir = ".";

                if (GetOpenFileNameA(&ofn))
                {
                    // Exception thrown at 0x00007FF85E2FFABC (KernelBase.dll) in DirectX12Study_240709.exe:
                    // 0x000006BA: RPC 서버를 사용할 수 없습니다. 수정 필요

                    std::string wStrFile = ofn.lpstrFile;
                    size_t idx           = wStrFile.rfind("\\");

                    m_openModelFileBasePath.assign(wStrFile.begin(), wStrFile.begin() + idx + 1);
                    m_openModelFileName.assign(wStrFile.begin() + idx + 1, wStrFile.end());
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                char lpstrFile[100] = "";

                OPENFILENAMEA ofn = {};
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize     = sizeof(ofn);
                ofn.hwndOwner       = m_hwnd;
                ofn.lpstrFilter     = "모든 파일\0*.*";
                ofn.lpstrFile       = lpstrFile;
                ofn.nMaxFile        = 100;
                ofn.lpstrInitialDir = ".";

                if (GetSaveFileNameA(&ofn))
                {
                    // Exception thrown at 0x00007FF85E2FFABC (KernelBase.dll) in DirectX12Study_240709.exe:
                    // 0x000006BA: RPC 서버를 사용할 수 없습니다. 수정 필요
                    SaveFile(ofn.lpstrFile);
                }
            }
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
        // static float speed = 0.0f;
        // ImGui::SliderFloat("Character front speed", &speed, 0.0001f, 0.001f, "%.7f");
        // m_model->SetSpeed(speed, FRONT);
    }
    // Mouse & keyboard
    if (ImGui::CollapsingHeader("Inputs"))
    {
        ImGui::Text("Mouse Xpos: %.3f", m_mouseX);
        ImGui::Text("Mouse Ypos: %.3f", m_mouseY);
        ImGui::Text("Eye position: %.1f, %1f, %1f", m_globalConstsData.eyeWorld.x, m_globalConstsData.eyeWorld.y,
                    m_globalConstsData.eyeWorld.z);
    }
    // Mouse & keyboard
    if (ImGui::CollapsingHeader("Animation list"))
    {
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0));

            // 100 => animation size로 대체
            for (int i = 0; i < m_animClips.size(); i++)
            {
                // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                if (ImGui::Selectable(m_animClips[i].c_str(), selected == i))
                    selected = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view",
                              ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("MyObject: %d", selected);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Description"))
                {
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
                                       "incididunt ut labore et dolore magna aliqua. ");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Details"))
                {
                    ImGui::Text("ID: 0123456789");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("Play"))
            {
                m_aniPlayFlag  = true;
                m_selectedAnim = selected;
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                m_aniPlayFlag  = false;
                m_selectedAnim = 0;
            }
            ImGui::EndGroup();
        }
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

void ModelViewer::InitLights()
{
    AppBase::InitLights();
}

void ModelViewer::UpdateLights()
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
        //l->Update();
    }
}

void ModelViewer::ChangeModel()
{
    if (!m_openModelFileName.empty())
    {
        //// SAFE_DELETE(m_model);

        // m_model = new Model;

        // auto [model, material] =
        //     GeometryGenerator::ReadFromModelFile(m_openModelFileBasePath.c_str(), m_openModelFileName.c_str());

        //// model을 재사용 할 수 있도록 수정
        // m_model->Initialize(m_device, m_commandList, model, material);
        // m_model->GetMaterialConstCPU().albedoFactor = XMFLOAT3(0.0f, 1.0f, 0.0f);
        // m_model->GetMaterialConstCPU().useAlbedoMap = false;
        // m_model->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));

        // WaitForPreviousFrame();

        // m_openModelFileName.clear();
    }
}
// 엔진에서 쓰일 모델 정보.
void ModelViewer::SaveFile(const char *filename)
{
    FILE *fp = nullptr;

    fopen_s(&fp, filename, "wt");
    if (!fp)
    {
        return;
    }

    fprintf(fp, m_basPath.c_str());
    fprintf(fp, "\n");
    for (auto &e : m_animClips)
    {
        fprintf(fp, e.c_str());
        fprintf(fp, "\n");
    }

    // 추가로 앞,뒤,옆 이동 속도 추가 (애니메이션과 싱크가 맞춰진 데이터)

    // 무기 장착 시나리오 추가.

    fclose(fp);
}
