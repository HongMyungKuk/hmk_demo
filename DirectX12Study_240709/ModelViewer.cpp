#include "pch.h"

#include "Camera.h"
#include "GeometryGenerator.h"
#include "Input.h"
#include "Model.h"
#include "ModelViewer.h"
#include "SkinnedMeshModel.h"

ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    SAFE_DELETE(m_coordController);
    SAFE_DELETE(m_model);
    SAFE_DELETE(m_terrain);
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

    // Create the coordinate controller.
    {
        CREATE_MODEL_OBJ(m_coordController);
        {
            const float radius = 0.3f;

            MeshData sphere              = GeometryGenerator::MakeSphere(radius, 25, 25);
            sphere.albedoTextureFilename = "../../Asset/CoordTexture.png";
            m_coordController->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, {sphere});
            m_coordController->GetMaterialConstCPU().texFlag = true;
            m_coordController->UpdateWorldMatrix(Matrix::CreateTranslation(m_controllerCenter));

            // bounding sphere.
            m_boundingSphere = BoundingSphere(Vector3(m_controllerCenter), radius);
        }
    }

    WaitForPreviousFrame();

    // Create the model.
    {
        m_model = new SkinnedMeshModel;
        if (!m_model)
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

            ((SkinnedMeshModel *)m_model)
                ->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model, material, animData);
            m_model->GetMaterialConstCPU().texFlag = m_useTexture;
            m_model->GetMaterialConstCPU().ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_model->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        }
    }

    WaitForPreviousFrame();

    // Create the terrain
    {
        CREATE_MODEL_OBJ(m_terrain);
        {
            auto [model, _] = GeometryGenerator::ReadFromModelFile(m_basPath.c_str(), "3.fbx");
            m_terrain->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model);
            m_terrain->UpdateWorldMatrix(Matrix::CreateScale(10.0f, 10.0f, 10.0f) * Matrix::CreateTranslation(0.0f, -2.0f, 0.0f));
            m_terrain->GetMaterialConstCPU().diffuse = Vector3(1.0f);
            m_terrain->GetMaterialConstCPU().specular = Vector3(1.0f);
            m_terrain->GetMaterialConstCPU().texFlag = false;
        }
    }

    WaitForPreviousFrame();

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

void ModelViewer::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    ChangeModel();

    {
        m_coordController->GetMaterialConstCPU().diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
        m_coordController->Update();
    }

    // picking
    {
        if (m_leftButtonDown)
        {
            Vector4 ndcNear = Vector4(m_ndcX, m_ndcY, 0.0f, 1.0f);
            Vector4 ndcFar  = Vector4(m_ndcX, m_ndcY, 1.0f, 1.0f);

            auto viewRow = m_camera->GetViewMatrix();
            auto projRow = m_camera->GetProjectionMatrix();

            auto InverseViewProjection = (viewRow * projRow).Invert();

            auto worldNear = Vector4::Transform(ndcNear, InverseViewProjection);
            auto worldFar  = Vector4::Transform(ndcFar, InverseViewProjection);
            worldNear /= worldNear.w;
            worldFar /= worldFar.w;

            Vector3 worldNear3 = Vector3(worldNear);
            Vector3 worldFar3  = Vector3(worldFar);

            Vector3 dir = worldFar3 - worldNear3;
            dir.Normalize();

            SimpleMath::Ray curRay = SimpleMath::Ray(worldNear3, dir);
            float dist             = 0.0f;
            bool isPicking         = false;

            // 화면이 Imgui 크기 만큼 차지하고 있어 마우스의 ndc scale이 변경되었음.
            // 현재 수정 완료.
            m_boundingSphere.Center = Vector3::Transform(m_controllerCenter, viewRow.Invert());
            isPicking               = curRay.Intersects(m_boundingSphere, dist);
            if (isPicking)
            {
                Vector3 collesionPositon = worldNear3 + dist * dir;

                SimpleMath::Quaternion q;
                static Vector3 prevVector(0.0f);
                if (m_leftButtonDragStart)
                {
                    m_leftButtonDragStart = false;
                    prevVector            = collesionPositon - m_boundingSphere.Center;
                    prevVector.Normalize();
                }
                else
                {
                    Vector3 currentVector = collesionPositon - m_boundingSphere.Center;
                    currentVector.Normalize();

                    if ((currentVector - prevVector).Length() > 1e-5)
                    {
                        q          = SimpleMath::Quaternion::FromToRotation(prevVector, currentVector);
                        prevVector = currentVector;
                    }
                }

                auto translation = m_coordController->GetWorldRow().Translation();
                m_coordController->GetWorldRow().Translation(Vector3(0.0f));
                m_coordController->UpdateWorldMatrix(m_coordController->GetWorldRow() *
                                                     Matrix::CreateFromQuaternion(q) *
                                                     Matrix::CreateTranslation(translation));

                translation = m_model->GetWorldRow().Translation();
                m_model->GetWorldRow().Translation(Vector3(0.0f));
                m_model->UpdateWorldMatrix(m_model->GetWorldRow() * Matrix::CreateFromQuaternion(q) *
                                           Matrix::CreateTranslation(translation));
            }
        }
    }

    m_model->Update();

    SkinnedMeshModel *model = dynamic_cast<SkinnedMeshModel *>(m_model);

    if (model != nullptr)
    {
        if (((SkinnedMeshModel *)m_model)->GetAnim().clips.size() > 0)
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

                if (GameInput::IsPressed(GameInput::kKey_up))
                {
                    state = 1;
                    m_model->MoveFront(dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_right))
                {
                    state = 2;
                    m_model->MoveRight(dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_left))
                {
                    state = 3;
                    m_model->MoveLeft(dt);
                }
                else if (GameInput::IsPressed(GameInput::kKey_down))
                {
                    state = 4;
                    m_model->MoveBack(dt);
                }
                else
                {
                    state = 0;
                }

                if (m_aniPlayFlag)
                    state = m_selectedAnim;

                ((SkinnedMeshModel *)m_model)->UpdateAnimation(state, frameCount++);
            }
        }
    }

    // etc...
    {
        m_box->Update();
        m_ground->Update();
    }
}

void ModelViewer::Render()
{
    AppBase::Render();

    m_commandList->SetPipelineState(Graphics::blendCoverPSO);
    m_coordController->Render(m_commandList);

    m_commandList->SetPipelineState(m_model->GetPSO(m_isWireFrame));
    m_model->Render(m_commandList);

    // m_commandList->SetPipelineState(m_box->GetPSO());
    // m_box->Render(m_device, m_commandList);

    m_commandList->SetPipelineState(m_ground->GetPSO(m_isWireFrame));
    m_ground->Render(m_commandList);

    if (m_drawAsNormal)
    {
        // Set normal PSO
        m_commandList->SetPipelineState(Graphics::normalPSO);
        m_model->RenderNormal(m_commandList);
    }
}

void ModelViewer::UpdateGui(const float frameRate)
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

    ImGui::End();
}

void ModelViewer::ChangeModel()
{
    if (!m_openModelFileName.empty())
    {
        SAFE_DELETE(m_model);

        m_model = new Model;

        auto [model, material] =
            GeometryGenerator::ReadFromModelFile(m_openModelFileBasePath.c_str(), m_openModelFileName.c_str());

        // model을 재사용 할 수 있도록 수정
        m_model->Initialize(m_device, m_commandList, m_commandAllocator, m_commandQueue, model, material);
        m_model->GetMaterialConstCPU().ambient = XMFLOAT3(0.0f, 1.0f, 0.0f);
        m_model->GetMaterialConstCPU().texFlag = false;
        m_model->UpdateWorldMatrix(XMMatrixTranslation(0.0f, 0.5f, 0.0f));

        WaitForPreviousFrame();

        m_openModelFileName.clear();
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
