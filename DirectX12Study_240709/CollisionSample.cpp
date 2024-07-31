#include "pch.h"

#include "Camera.h"
#include "CollisionSample.h"
#include "Command.h"
#include "GeometryGenerator.h"
#include "Input.h"
#include "Model.h"

bool CollisionSample::Initialize()
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

    // 구 생성
    {
        const float radius = 0.5f;
        Vector3 center(0.0f);
        MeshData sphere = GeometryGenerator::MakeSphere(radius, 25, 25);
        auto obj        = new Model;
        obj->Initialize(m_device, m_commandList, {sphere});
        obj->SetPos(center);
        obj->GetMaterialConstCPU().albedoFactor = m_obj1Color;
        obj->UpdateWorldMatrix(Matrix::CreateScale(Vector3(1.0f)));
        m_opaqueList.push_back(obj);

        m_sphereCollider.radius = radius;
        m_sphereCollider.center = center;
    }

    // 사각형 생성
    {
        Vector3 normal(0.0f, 0.0f, -1.0f);
        Vector3 center(0.0f, -3.0f, 3.0f);
        MeshData square = GeometryGenerator::MakeSquare(5.0f, 5.0f);
        auto obj        = new Model;
        obj->Initialize(m_device, m_commandList, {square});
        obj->SetPos(center);
        obj->GetMaterialConstCPU().albedoFactor = m_obj2Color;
        obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
        m_opaqueList.push_back(obj);

        m_planeCollider.normal = normal;
        m_planeCollider.center = center;
    }

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForPreviousFrame();

    // Set event handler.
    g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::FRONT,
                                           new ObjectMoveFrontCommand(m_opaqueList[0], m_light));
    g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::BACK,
                                           new ObjectMoveBackCommand(m_opaqueList[0], m_light));
    g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::RIGHT,
                                           new ObjectMoveRightCommand(m_opaqueList[0], m_light));
    g_EvnetHandler.RegistObjectMoveCommand(EventHandler::OBJ_COMMAND_TYPE::LEFT,
                                           new ObjectMoveLeftCommand(m_opaqueList[0], m_light));

    return true;
}

void CollisionSample::UpdateGui(const float frameRate)
{
    AppBase::UpdateGui(frameRate);

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

void CollisionSample::Render()
{
    AppBase::Render();

    m_commandList->RSSetViewports(1, &Graphics::mainViewport);
    m_commandList->RSSetScissorRects(1, &Graphics::mainSissorRect);

    // Root signature 이후에 변경 .... 방법 찾기
    m_commandList->SetGraphicsRootSignature(Graphics::defaultRootSignature);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_globalConstsBuffer.GetResource()->GetGPUVirtualAddress());

    m_commandList->SetGraphicsRootDescriptorTable(3, Graphics::s_Texture[1]);
}

void CollisionSample::Update(const float dt)
{
    AppBase::Update(dt);

    UpdateCamera(dt);

    {
        if (GameInput::IsPressed(GameInput::kKey_up))
        {
            if (m_moveFlag[0])
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::FRONT, dt);
        }
        else if (GameInput::IsPressed(GameInput::kKey_right))
        {
            if (m_moveFlag[1])
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::RIGHT, dt);
        }
        else if (GameInput::IsPressed(GameInput::kKey_left))
        {
            if (m_moveFlag[2])
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::LEFT, dt);
        }
        else if (GameInput::IsPressed(GameInput::kKey_down))
        {
            if (m_moveFlag[3])
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::BACK, dt);
        }
        else {}

        m_sphereCollider.center = m_opaqueList[0]->GetPos();
    }

    // Check collision
    {
        auto radius = m_sphereCollider.radius;
        auto p      = m_sphereCollider.center;
        auto n      = m_planeCollider.normal;
        auto c      = m_planeCollider.center;

        auto denom = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
        auto numer = std::abs(n.x * (p.x - c.x) + n.y * (p.y - c.y) + n.z * (p.z - c.z));

        auto d = numer / denom;

        if (d <= radius)
        {
            m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
            m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collisionColor;

            m_moveFlag[3] = false;
        }
        else
        {
            m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_obj1Color;
            m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_obj2Color;

            m_moveFlag[3] = true;
        }


        // ray picking.
        // sphere intersection.

        if (m_leftButtonDown)
        {
            // std::cout << m_ndcX << " " << m_ndcY << std::endl;


            Vector3 mouseNdcNear = Vector3(m_ndcX, m_ndcY, 0.0f);
            Vector3 mouseNdxFar  = Vector3(m_ndcX, m_ndcY, 1.0f);

            Matrix inverseViewPoj = m_globalConstsData.viewProjInv.Transpose();

            Vector3 mouseWorldNear = Vector3::Transform(mouseNdcNear, inverseViewPoj);
            Vector3 mouseWorldFar  = Vector3::Transform(mouseNdxFar, inverseViewPoj);

            Vector3 rayDir = mouseWorldFar - mouseWorldNear;
            rayDir.Normalize();

            Vector3 gcDir = m_sphereCollider.center - mouseWorldNear;

            float b = rayDir.Dot(gcDir);
            float c = gcDir.Dot(gcDir) - m_sphereCollider.radius * m_sphereCollider.radius;

            float d = b * b - c;

            std::cout << d << std::endl;

            if (d >= 0.0f)
            {
                m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
            }
        }

        // https://stackoverflow.com/questions/73866852/ray-cylinder-intersection-formula
        // ray cylinder collision.
    }

    m_postProcess.GetConstCPU().exposure     = m_exposureFactor;
    m_postProcess.GetConstCPU().gammeaFactor = m_gammaFactor;
    m_postProcess.Update();
}

void CollisionSample::UpdateLights()
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
