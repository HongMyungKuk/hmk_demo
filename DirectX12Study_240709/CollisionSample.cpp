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
        const float radius = 0.1f;
        Vector3 center(0.0f, 5.0f, 0.0f);
        MeshData sphere = GeometryGenerator::MakeSphere(radius, 25, 25);
        auto obj        = new Model;
        obj->Initialize(m_device, m_commandList, {sphere});
        obj->SetPos(center);
        obj->GetMaterialConstCPU().albedoFactor = m_obj1Color;
        obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
        m_opaqueList.push_back(obj);

        m_sphereCollider.radius = radius;
        m_sphereCollider.center = center;

        m_boundingSphere = DirectX::BoundingSphere(center, radius);
    }

    //// 삼각형 생성
    //{
    //    Vector3 normal(0.0f, 1.0f, 0.0f);
    //    Vector3 center(0.0f, 0.0f, 0.0f);
    //    MeshData triangle = GeometryGenerator::MakeTriangle(5.0f);
    //    auto obj          = new Model;
    //    obj->Initialize(m_device, m_commandList, {triangle});
    //    obj->SetPos(center);
    //    obj->GetMaterialConstCPU().albedoFactor = m_obj2Color;
    //    obj->UpdateWorldMatrix(Matrix::CreateRotationX(XM_PIDIV2));
    //    m_opaqueList.push_back(obj);

    //    m_triangleCollider.normal = normal;
    //    for (auto v : triangle.vertices)
    //    {
    //        v.position = Vector3::Transform(v.position, Matrix::CreateRotationX(XM_PIDIV2));
    //        m_triangleCollider.positions.push_back(v.position);
    //    }
    //}

    // 격자 생성
    {
        Vector3 normal(0.0f, 1.0f, 0.0f);
        Vector3 center(0.0f, 0.0f, 0.0f);
        MeshData grid = GeometryGenerator::MakeSquareGrid(25, 25, 50.0f, Vector2(1.0f));
        auto obj      = new Model;
        obj->Initialize(m_device, m_commandList, {grid});
        obj->SetPos(center);
        obj->GetMaterialConstCPU().albedoFactor = m_obj2Color;
        obj->UpdateWorldMatrix(Matrix::CreateRotationX(XM_PIDIV2));
        m_opaqueList.push_back(obj);

        m_triangleCollider.normal = normal;
        for (auto v : grid.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateRotationX(XM_PIDIV2));
            m_triangleCollider.positions.push_back(v.position);
        }

        for (auto i : grid.indices)
        {
            m_triangleCollider.indices.push_back(i);
        }
    }

    //// Collider 생성
    //{
    //    Vector3 v0 = Vector3(-3.0f, -3.0f, -3.0f);
    //    Vector3 v1 = Vector3(-3.0f, 3.0f, 0.0f);
    //    Vector3 v2 = Vector3(3.0f, -3.0f, -3.0f);
    //    m_collider.Initialize(0.1f, v0, v1, v2);

    //    Model *obj = new Model;
    //    obj->Initialize(m_device, m_commandList, m_collider.m_meshes);
    //    obj->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
    //    m_opaqueList.push_back(obj);
    //}

    // 실린더 생성
    {
        // float radius      = 1.0f;
        // Vector3 center    = Vector3(0.0f, 0.0f, -2.0f);
        // MeshData cylinder = GeometryGenerator::MakeCylinder(radius, radius, 2.0f, 25);

        // auto obj = new Model;
        // obj->Initialize(m_device, m_commandList, {m_collider.m_cylinder});
        // obj->GetMaterialConstCPU().albedoFactor = m_obj3Color;
        //// obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
        // m_opaqueList.push_back(obj);

        // m_cylinderCollider.radius       = 1.0f;
        // m_cylinderCollider.height       = 2.0f;
        // Vector3 v0                      = cylinder.vertices[0].position;
        // Vector3 v1                      = Vector3::Transform(v0, Matrix::CreateRotationY(XM_PI));
        // m_cylinderCollider.bottomCenter = (v0 + v1) * 0.5f;
        // m_cylinderCollider.topCenter =
        //     Vector3::Transform(m_cylinderCollider.bottomCenter, Matrix::CreateTranslation(0.0f, 2.0f, 0.0f));

        // m_cylinderCollider.bottomCenter =
        //     Vector3::Transform(m_cylinderCollider.bottomCenter, Matrix::CreateTranslation(center));
        // m_cylinderCollider.topCenter =
        //     Vector3::Transform(m_cylinderCollider.topCenter, Matrix::CreateTranslation(center));

        // m_cylinderCollider = m_collider.m_cylinderCollider[2];
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

    Vector3 prevPos = m_opaqueList[0]->GetPos();
    Vector3 curPos  = prevPos;
    {
        if (GameInput::IsPressed(GameInput::kKey_up))
        {
            if (m_moveFlag[0])
            {
                m_opaqueList[0]->AddVelocity(Vector3(0.0f, 0.0f, 1.0f));
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_right))
        {
            if (m_moveFlag[1])
            {
                m_opaqueList[0]->AddVelocity(Vector3(1.0f, 0.0f, 0.0f));
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_left))
        {
            if (m_moveFlag[2])
            {
                m_opaqueList[0]->AddVelocity(Vector3(-1.0f, 0.0f, 0.0f));
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_down))
        {
            if (m_moveFlag[3])
            {
                m_opaqueList[0]->AddVelocity(Vector3(0.0f, 0.0f, -1.0f));
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_z))
        {

            m_opaqueList[0]->AddVelocity(Vector3(0.0f, 1.0f, 0.0f));
        }
        if (GameInput::IsPressed(GameInput::kKey_x))
        {

            m_opaqueList[0]->AddVelocity(Vector3(0.0f, -1.0f, 0.0f));
        }

        // 움직이고.
        m_opaqueList[0]->Move(dt);
        curPos = m_opaqueList[0]->GetPos();

        m_sphereCollider.center = m_opaqueList[0]->GetPos();
    }

    m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_obj1Color;

    //// Check collision
    //{
    //    auto radius = m_sphereCollider.radius;
    //    auto p      = m_sphereCollider.center;
    //    auto n      = m_planeCollider.normal;
    //    auto c      = m_planeCollider.center;

    //    auto denom = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    //    auto numer = std::abs(n.x * (p.x - c.x) + n.y * (p.y - c.y) + n.z * (p.z - c.z));

    //    auto d = numer / denom;

    //    if (d <= radius)
    //    {
    //        m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
    //        m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collisionColor;

    //        m_moveFlag[3] = false;
    //    }
    //    else
    //    {
    //        m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_obj1Color;
    //        m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_obj2Color;
    //        m_opaqueList[2]->GetMaterialConstCPU().albedoFactor = m_obj3Color;

    //        m_moveFlag[3] = true;
    //    }

    //    m_distList["sphere"]   = 1000.0f;
    //    m_distList["triangle"] = 1000.0f;
    //    m_distList["cylinder"] = 1000.0f;

    //    if (m_leftButtonDown)
    //    {
    //        // 구 충돌
    //        Vector3 mouseNdcNear = Vector3(m_ndcX, m_ndcY, 0.0f);
    //        Vector3 mouseNdxFar  = Vector3(m_ndcX, m_ndcY, 1.0f);

    //        Matrix inverseViewPoj = m_globalConstsData.viewProjInv.Transpose();

    //        Vector3 mouseWorldNear = Vector3::Transform(mouseNdcNear, inverseViewPoj);
    //        Vector3 mouseWorldFar  = Vector3::Transform(mouseNdxFar, inverseViewPoj);

    //        Vector3 d = mouseWorldFar - mouseWorldNear;
    //        d.Normalize();
    //        Vector3 o = mouseWorldNear;
    //        Vector3 s = m_sphereCollider.center;

    //        float b = o.Dot(d) - s.Dot(d);
    //        float c = (o - s).Dot(o - s) - m_sphereCollider.radius * m_sphereCollider.radius;

    //        float det = b * b - c;

    //        if (det >= 0.0f)
    //        {
    //            float t1 = -b + sqrtf(det);
    //            float t2 = -b - sqrtf(det);

    //            m_distList["sphere"] = XMMin((t1 * d).Length(), (t2 * d).Length());

    //            m_collisionFlag[0] = true;
    //        }

    //        // 삼각형 충돌
    //        Vector3 n = m_triangleCollider.normal;

    //        // Vector3 v0 = m_triangleCollider.positions[0];
    //        // Vector3 v1 = m_triangleCollider.positions[1];
    //        // Vector3 v2 = m_triangleCollider.positions[2];

    //        // float numer = v0.Dot(n) - o.Dot(n);
    //        // float denom = n.Dot(d);

    //        float t = numer / denom;

    //        p = o + t * d; // closest hit position.

    //        /*  Vector3 a1 = (p - v0).Cross(v2 - v0);
    //          Vector3 a2 = (p - v1).Cross(v0 - v1);
    //          Vector3 a3 = (p - v2).Cross(v1 - v2);

    //          a1 /= a1.Length();
    //          a2 /= a2.Length();
    //          a3 /= a3.Length();

    //          float alpha1 = a1.Dot(n);
    //          float alpha2 = a2.Dot(n);
    //          float alpha3 = a3.Dot(n);

    //          if (alpha1 >= 0.0f && alpha2 >= 0.0f && alpha3 >= 0.0f)
    //          {
    //              m_distList["triangle"] = (t * d).Length();

    //              m_collisionFlag[1] = true;
    //          }*/

    //        // 실린더 충돌
    //        Vector3 tc = m_cylinderCollider.topCenter;
    //        Vector3 bc = m_cylinderCollider.bottomCenter;
    //        Vector3 x  = o - tc;
    //        Vector3 y  = o - bc;
    //        Vector3 v  = m_cylinderCollider.bottomCenter - m_cylinderCollider.topCenter;
    //        v.Normalize();

    //        float a = d.Dot(d) - d.Dot(v) * d.Dot(v);
    //        c       = x.Dot(x) - x.Dot(v) * x.Dot(v) - m_cylinderCollider.radius * m_cylinderCollider.radius;
    //        b       = 2.0f * (d.Dot(x) - d.Dot(v) * x.Dot(v));

    //        det = b * b - 4.0f * a * c;

    //        std::cout << det << std::endl;

    //        float t1 = (-b + sqrtf(det)) / (2.0f * a);
    //        float t2 = (-b - sqrtf(det)) / (2.0f * a);

    //        t = 0.0f;
    //        if ((t1 * d).Length() > (t2 * d).Length())
    //        {
    //            t = t2;
    //        }
    //        else
    //        {
    //            t = t1;
    //        }

    //        p = o + d * t;

    //        float m = (p - tc).Dot(v);

    //        if (det >= 0 && m >= 0 && m <= m_cylinderCollider.height)
    //        {
    //            m_distList["cylinder"] = XMMin((t1 * d).Length(), (t2 * d).Length());
    //            m_collisionFlag[2]     = true;
    //        }
    //    }

    //    // 충돌 후 동작
    //    {
    //        // dist 가 제일 가까운것만 충돌처리하면 된다.
    //        float minDist = D3D12_FLOAT32_MAX;

    //        for (const auto &ele : m_distList)
    //        {
    //            minDist = XMMin(minDist, ele.second);
    //        }

    //        if (minDist != 1000.0f)
    //        {
    //            if (minDist == m_distList["cylinder"] && m_collisionFlag[2] == true)
    //            {
    //                m_opaqueList[2]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
    //            }
    //            if (minDist == m_distList["triangle"] && m_collisionFlag[1] == true)
    //            {
    //                m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
    //            }
    //            if (minDist == m_distList["sphere"] && m_collisionFlag[0] == true)
    //            {
    //                m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
    //            }
    //        }

    //        m_distList["sphere"]   = 1000.0f;
    //        m_distList["triangle"] = 1000.0f;
    //        m_distList["cylinder"] = 1000.0f;

    //        m_collisionFlag[2] = false;
    //        m_collisionFlag[1] = false;
    //        m_collisionFlag[0] = false;
    //    }

    //    // https://stackoverflow.com/questions/73866852/ray-cylinder-intersection-formula
    //    // ray cylinder collision.
    //}

    // if (m_leftButtonDown)
    //{
    //     // 구 충돌
    //     Vector3 mouseNdcNear = Vector3(m_ndcX, m_ndcY, 0.0f);
    //     Vector3 mouseNdxFar  = Vector3(m_ndcX, m_ndcY, 1.0f);

    //    Matrix inverseViewPoj = m_globalConstsData.viewProjInv.Transpose();

    //    Vector3 mouseWorldNear = Vector3::Transform(mouseNdcNear, inverseViewPoj);
    //    Vector3 mouseWorldFar  = Vector3::Transform(mouseNdxFar, inverseViewPoj);

    //    Vector3 rayDir = mouseWorldFar - mouseWorldNear;
    //    rayDir.Normalize();

    //    Ray ray = Ray(rayDir, mouseWorldNear);

    //    if (m_collider.CheckRayIntersect(ray))
    //    {
    //        m_collider.BehaviorAfterCollsion();

    //        m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
    //    }
    //}

    //// ray 가 생겼는지 여부를 체크
    // Vector3 rayDir        = curPos - prevPos;
    // const float rayLength = rayDir.Length();
    // rayDir.Normalize();

    // Ray ray = Ray(rayDir, prevPos);

    //// 1. 물체의 이동에 따른 ray와 collider 의 작용

    //// 현재 움직이고 있는 상태일때 ray가 발생한다.
    // if (rayLength >= 1e-5)
    //{
    //     // 충돌할 물체와 멀어지는 방향으로는 충돌체크를 제외한다.
    //     if (m_collider.m_triangleCollider.normal.Dot(rayDir) < 0.0f)
    //     {
    //         // 충돌 할 물체가 있는가 체크
    //         if (m_collider.CheckRayIntersect(ray) && m_testFlag)
    //         {
    //             // 한번만 계산되어야 함
    //             m_testFlag = false;
    //         }
    //     }
    //     else if (m_collider.m_triangleCollider.normal.Dot(rayDir) == 0.0f)
    //     {
    //         // m_moveFlag[0] = true;
    //     }
    //     else
    //     {
    //         m_testCollisionFlag = false;
    //     }
    // }
    //// 움직이지 않고 있을때
    // else
    //{
    //     // 충돌 중이 아닐때
    //     if (!m_testCollisionFlag)
    //     {
    //         m_testFlag = true;
    //         // m_moveFlag[0] = true;
    //     }
    // }

    // float minX = XMMin(prevPos.x, curPos.x);
    // float minY = XMMin(prevPos.y, curPos.y);
    // float minZ = XMMin(prevPos.z, curPos.z);
    // float maxX = XMMax(prevPos.x, curPos.x);
    // float maxY = XMMax(prevPos.y, curPos.y);
    // float maxZ = XMMax(prevPos.z, curPos.z);

    //// 2. 1번의 상호작용을 토대로 실제 충돌처리

    // if (((rayLength >= 1e-5) && (m_collider.hitPostion.x <= maxX && m_collider.hitPostion.x >= minX) &&
    //      (m_collider.hitPostion.y <= maxY && m_collider.hitPostion.y >= minY) &&
    //      (m_collider.hitPostion.z <= maxZ && m_collider.hitPostion.z >= minZ)) ||
    //     m_testCollisionFlag)
    //{

    //    Vector3 translation = Vector3(0.0f);
    //    Vector3 v           = m_opaqueList[0]->GetVelocity();
    //    Vector3 slopeVector = v;

    //    // 충돌이 일어날 가능성을 다시 체크
    //    // 조건을 체크 안하면 hitposition 이 infinite 가 됨.
    //    if (rayLength >= 1e-5 && m_collider.CheckRayIntersect(ray) &&
    //        m_collider.m_triangleCollider.normal.Dot(rayDir) < 0.0f)
    //    {
    //        m_collider.BehaviorAfterCollsion();
    //        m_testCollisionFlag                                 = true;
    //        m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
    //        // m_moveFlag[0]                                       = false;

    //        translation = m_collider.hitPostion - curPos;

    //        float vProjN      = v.Dot(m_collider.m_triangleCollider.normal);
    //        Vector3 invNormal = vProjN * m_collider.m_triangleCollider.normal;
    //        slopeVector       = (v - invNormal);
    //    }

    //    // hit 된 위치로 되돌림
    //    m_opaqueList[0]->SetPos(m_opaqueList[0]->GetPos() + translation);
    //    m_opaqueList[0]->UpdateWorldMatrix(m_opaqueList[0]->GetWorldRow() * Matrix::CreateTranslation(translation));

    //    // 충돌하는 동안은 새로운 속도 벡터 설정

    //    m_opaqueList[0]->SetVelocity(slopeVector);
    //    m_opaqueList[0]->Move(dt);
    //}
    // else
    //{
    //    prevPos = curPos;
    //}

    // 속도 초기화.
    // m_opaqueList[0]->SetVelocity(Vector3(0.0f));

    if (m_gravityFlag == true)
    {
        // 중력
        float m   = 1.0f;
        Vector3 a = Vector3(0.0f, -9.8f, 0.0f);

        m_opaqueList[0]->AddVelocity(a * dt * 0.0001f);
        m_opaqueList[0]->Move(dt);
    }

    m_boundingSphere.Center = m_opaqueList[0]->GetPos();

    bool hit = false;

    // 물체 하나면 상관 없지만, 모든 물체에 대해서 맵을 충돌처리 하는 비용은 크다.

    for (uint32_t i = 0; i < m_triangleCollider.indices.size(); i += 3)
    {
        if (m_boundingSphere.Intersects(m_triangleCollider.positions[m_triangleCollider.indices[i]],
                                        m_triangleCollider.positions[m_triangleCollider.indices[i + 1]],
                                        m_triangleCollider.positions[m_triangleCollider.indices[i + 2]]))
        {
            hit = true;
            break;
        }
    }

    // 둟는 현상 발생
    if (hit)
    {
        m_opaqueList[0]->GetMaterialConstCPU().albedoFactor = m_collisionColor;
        m_opaqueList[0]->SetVelocity(Vector3(0.0f));
        m_gravityFlag = false;
    }
    else
    {
        m_gravityFlag = true;
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

void TransformTriangleColiider::Initialize(const float radius, const Vector3 v0, const Vector3 v1, const Vector3 v2)
{
    m_radius = radius;

    // sphere x3
    {
        // set render mesh.
        MeshData sphere = GeometryGenerator::MakeSphere(radius, 25, 25);
        for (auto &v : sphere.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v0));
        }
        // m_meshes.push_back(sphere);
        for (auto &v : sphere.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v1 - v0));
        }
        // m_meshes.push_back(sphere);
        for (auto &v : sphere.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v2 - v1));
        }
        // m_meshes.push_back(sphere);

        // set collider.
        m_sphereCollider[0].radius = radius;
        m_sphereCollider[0].center = v0;

        m_sphereCollider[1].radius = radius;
        m_sphereCollider[1].center = v1;

        m_sphereCollider[2].radius = radius;
        m_sphereCollider[2].center = v2;
    }

    // triangle
    {
        // set render mesh.
        MeshData triangle = GeometryGenerator::MakeTriangle(v0, v1, v2);
        m_meshes.push_back(triangle);
        for (auto &v : triangle.vertices)
        {
            v.position =
                Vector3::Transform(v.position, Matrix::CreateTranslation(triangle.vertices[0].normal * radius));
        }
        // m_meshes.push_back(triangle);

        // set collider.
        m_triangleCollider.normal = triangle.vertices[0].normal;
        for (auto p : triangle.vertices)
        {
            m_triangleCollider.positions.push_back(p.position); // v0 > v1 > v2
        }
    }

    // cylinder x3
    {
        // cylinder #1
        MeshData cylinder = GeometryGenerator::MakeCylinder(radius, radius, (v2 - v1).Length(), 25);
        auto p0           = cylinder.vertices[0].position;
        auto p1           = Vector3::Transform(p0, Matrix::CreateRotationY(XM_PI));
        auto centerBottom = (p0 + p1) * 0.5f;
        auto centerTop =
            Vector3::Transform(centerBottom, Matrix::CreateTranslation(Vector3(0.0f, (v2 - v1).Length(), 0.0f)));
        auto q = Quaternion::FromToRotation(Vector3(0.0f, 1.0f, 0.0f), v1 - v2);
        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateFromQuaternion(q));
        }
        centerBottom = Vector3::Transform(centerBottom, Matrix::CreateFromQuaternion(q));
        centerTop    = Vector3::Transform(centerTop, Matrix::CreateFromQuaternion(q));

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v2 - centerBottom));
        }
        // m_meshes.push_back(cylinder);
        Vector3 newCenterBottom = Vector3::Transform(centerBottom, Matrix::CreateTranslation(v2 - centerBottom));
        Vector3 newCenterTop    = Vector3::Transform(centerTop, Matrix::CreateTranslation(v2 - centerBottom));

        m_cylinderCollider[0].bottomCenter = centerBottom;
        m_cylinderCollider[0].topCenter    = centerTop;
        m_cylinderCollider[0].radius       = radius;
        m_cylinderCollider[0].height       = (v2 - v1).Length();

        // cylinder #2
        cylinder     = GeometryGenerator::MakeCylinder(radius, radius, (v1 - v0).Length(), 25);
        p0           = cylinder.vertices[0].position;
        p1           = Vector3::Transform(p0, Matrix::CreateRotationY(XM_PI));
        centerBottom = (p0 + p1) * 0.5f;
        centerTop =
            Vector3::Transform(centerBottom, Matrix::CreateTranslation(Vector3(0.0f, (v1 - v0).Length(), 0.0f)));
        q = Quaternion::FromToRotation(Vector3(0.0f, 1.0f, 0.0f), v1 - v0);
        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateFromQuaternion(q));
        }
        centerBottom = Vector3::Transform(centerBottom, Matrix::CreateFromQuaternion(q));
        centerTop    = Vector3::Transform(centerTop, Matrix::CreateFromQuaternion(q));

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v0 - centerBottom));
        }
        // m_meshes.push_back(cylinder);
        newCenterBottom = Vector3::Transform(centerBottom, Matrix::CreateTranslation(v0 - centerBottom));
        newCenterTop    = Vector3::Transform(centerTop, Matrix::CreateTranslation(v0 - centerBottom));

        m_cylinderCollider[1].bottomCenter = newCenterBottom;
        m_cylinderCollider[1].topCenter    = newCenterTop;
        m_cylinderCollider[1].radius       = radius;
        m_cylinderCollider[1].height       = (v1 - v0).Length();

        // cylinder #3
        cylinder     = GeometryGenerator::MakeCylinder(radius, radius, (v0 - v2).Length(), 25);
        p0           = cylinder.vertices[0].position;
        p1           = Vector3::Transform(p0, Matrix::CreateRotationY(XM_PI));
        centerBottom = (p0 + p1) * 0.5f;
        centerTop =
            Vector3::Transform(centerBottom, Matrix::CreateTranslation(Vector3(0.0f, (v0 - v2).Length(), 0.0f)));
        q = Quaternion::FromToRotation(Vector3(0.0f, 1.0f, 0.0f), v0 - v2);
        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateFromQuaternion(q));
        }
        centerBottom = Vector3::Transform(centerBottom, Matrix::CreateFromQuaternion(q));
        centerTop    = Vector3::Transform(centerTop, Matrix::CreateFromQuaternion(q));

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v2 - centerBottom));
        }
        // m_meshes.push_back(cylinder);
        newCenterBottom = Vector3::Transform(centerBottom, Matrix::CreateTranslation(v2 - centerBottom));
        newCenterTop    = Vector3::Transform(centerTop, Matrix::CreateTranslation(v2 - centerBottom));

        m_cylinderCollider[2].bottomCenter = newCenterBottom;
        m_cylinderCollider[2].topCenter    = newCenterTop;
        m_cylinderCollider[2].radius       = radius;
        m_cylinderCollider[2].height       = (v0 - v2).Length();
    }
}
