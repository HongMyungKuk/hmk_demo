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
        Vector3 center(0.0f, 0.0f, -2.0f);
        MeshData sphere = GeometryGenerator::MakeSphere(radius, 25, 25);
        auto obj        = new Model;
        obj->Initialize(m_device, m_commandList, {sphere});
        obj->SetPos(center);
        obj->GetMaterialConstCPU().albedoFactor = m_obj1Color;
        obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
        m_opaqueList.push_back(obj);

        m_sphereCollider.radius = radius;
        m_sphereCollider.center = center;
    }

    //// 삼각형 생성
    //{
    //    Vector3 normal(0.0f, 0.0f, -1.0f);
    //    Vector3 center(0.0f, 0.0f, 3.0f);
    //    MeshData triangle = GeometryGenerator::MakeTriangle(5.0f);
    //    auto obj          = new Model;
    //    obj->Initialize(m_device, m_commandList, {triangle});
    //    obj->SetPos(center);
    //    obj->GetMaterialConstCPU().albedoFactor = m_obj2Color;
    //    obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
    //    m_opaqueList.push_back(obj);

    //    m_triangleCollider.normal = normal;
    //    for (auto v : triangle.vertices)
    //    {
    //        v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(center));
    //        m_triangleCollider.positions.push_back(v.position);
    //    }
    //}

    //// 실린더 생성
    //{
    //    float radius      = 1.0f;
    //    Vector3 center    = Vector3(0.0f, 0.0f, -2.0f);
    //    MeshData cylinder = GeometryGenerator::MakeCylinder(radius, radius, 2.0f, 25);
    //    auto obj          = new Model;
    //    obj->Initialize(m_device, m_commandList, {cylinder});
    //    obj->GetMaterialConstCPU().albedoFactor = m_obj3Color;
    //    obj->UpdateWorldMatrix(Matrix::CreateTranslation(center));
    //    m_opaqueList.push_back(obj);

    //    m_cylinderCollider.radius       = 1.0f;
    //    m_cylinderCollider.height       = 2.0f;
    //    Vector3 v0                      = cylinder.vertices[0].position;
    //    Vector3 v1                      = Vector3::Transform(v0, Matrix::CreateRotationY(XM_PI));
    //    m_cylinderCollider.bottomCenter = (v0 + v1) * 0.5f;
    //    m_cylinderCollider.topCenter =
    //        Vector3::Transform(m_cylinderCollider.bottomCenter, Matrix::CreateTranslation(0.0f, 2.0f, 0.0f));

    //    m_cylinderCollider.bottomCenter =
    //        Vector3::Transform(m_cylinderCollider.bottomCenter, Matrix::CreateTranslation(center));
    //    m_cylinderCollider.topCenter =
    //        Vector3::Transform(m_cylinderCollider.topCenter, Matrix::CreateTranslation(center));
    //}

    // Collider 생성
    {
        Vector3 v0 = Vector3(-3.0f, -3.0f, 0.0f);
        Vector3 v1 = Vector3(-3.0f, 3.0f, 0.0f);
        Vector3 v2 = Vector3(3.0f, -3.0f, 0.0f);

        m_collider.Initialize(0.1f, v0, v1, v2);

        Model *obj = new Model;
        obj->Initialize(m_device, m_commandList, m_collider.m_meshes);
        obj->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
        m_opaqueList.push_back(obj);
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
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::FRONT, dt);
                curPos = m_opaqueList[0]->GetPos();
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_right))
        {
            if (m_moveFlag[1])
            {
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::RIGHT, dt);
                curPos = m_opaqueList[0]->GetPos();
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_left))
        {
            if (m_moveFlag[2])
            {
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::LEFT, dt);
                curPos = m_opaqueList[0]->GetPos();
            }
        }
        if (GameInput::IsPressed(GameInput::kKey_down))
        {
            if (m_moveFlag[3])
            {
                g_EvnetHandler.ObjectMoveHandle(EventHandler::OBJ_COMMAND_TYPE::BACK, dt);
                curPos = m_opaqueList[0]->GetPos();
            }
        }
        else {}

        m_sphereCollider.center = m_opaqueList[0]->GetPos();
    }

    m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = Vector3(0.2f, 0.2f, 0.6f);

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

    //    // ray picking.
    //    // sphere intersection.

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

    //        Vector3 v0 = m_triangleCollider.positions[0];
    //        Vector3 v1 = m_triangleCollider.positions[1];
    //        Vector3 v2 = m_triangleCollider.positions[2];

    //        float numer = v0.Dot(n) - o.Dot(n);
    //        float denom = n.Dot(d);

    //        float t = numer / denom;

    //        p = o + t * d; // closest hit position.

    //        Vector3 a1 = (p - v0).Cross(v2 - v0);
    //        Vector3 a2 = (p - v1).Cross(v0 - v1);
    //        Vector3 a3 = (p - v2).Cross(v1 - v2);

    //        a1 /= a1.Length();
    //        a2 /= a2.Length();
    //        a3 /= a3.Length();

    //        float alpha1 = a1.Dot(n);
    //        float alpha2 = a2.Dot(n);
    //        float alpha3 = a3.Dot(n);

    //        if (alpha1 >= 0.0f && alpha2 >= 0.0f && alpha3 >= 0.0f)
    //        {
    //            m_distList["triangle"] = (t * d).Length();

    //            m_collisionFlag[1] = true;
    //        }

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

    if (m_leftButtonDown)
    {
        // 구 충돌
        Vector3 mouseNdcNear = Vector3(m_ndcX, m_ndcY, 0.0f);
        Vector3 mouseNdxFar  = Vector3(m_ndcX, m_ndcY, 1.0f);

        Matrix inverseViewPoj = m_globalConstsData.viewProjInv.Transpose();

        Vector3 mouseWorldNear = Vector3::Transform(mouseNdcNear, inverseViewPoj);
        Vector3 mouseWorldFar  = Vector3::Transform(mouseNdxFar, inverseViewPoj);

        Vector3 rayDir = mouseWorldFar - mouseWorldNear;
        rayDir.Normalize();

        Ray ray = Ray(rayDir, mouseWorldNear);

        if (m_collider.CheckRayIntersect(ray))
        {

            m_collider.BehaviorAfterCollsion();

            m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
        }
    }

    // 곧 물체에 충돌할 가능성이 생김
    static float accDist      = 0.0f;
    static float hitDist      = 0.0f;
    static Vector3 prevRayDir = Vector3(0.0f);
    Vector3 curRayDir         = curPos - prevPos;

    if ((curPos - prevPos).Length() >= 1e-5)
    {
        curRayDir.Normalize();
        Ray ray = Ray(curRayDir, prevPos);
        // 초기에 한번 ray 충돌체크.
        if (prevRayDir == Vector3(0.0f))
        {
            // 충돌의 체크하고 플래그를 킴
            if (m_collider.CheckRayIntersect(ray))
            {
                hitDist = m_collider.hitDistance;

                m_collider.BehaviorAfterCollsion();

                m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
            }
        }
        // 새로운 충돌의 체크함
        else if (prevRayDir.Cross(curRayDir).Length() >= 1e-5)
        {
            if (m_collider.CheckRayIntersect(ray))
            {
                hitDist = m_collider.hitDistance;

                m_collider.BehaviorAfterCollsion();

                m_opaqueList[1]->GetMaterialConstCPU().albedoFactor = m_collider.m_color;
            }
            else
            {
                hitDist = m_collider.hitDistance;
            }
            accDist = 0.0f;
        }

        prevRayDir = curRayDir;
    }

    // 충돌의 거리의 계산으로 처리????
    {

        float deltaDist = (curPos - prevPos).Length();

        accDist += deltaDist;

        // 실체 충돌이 일어남.
        if (hitDist && abs(accDist - hitDist) <= 0.1f && abs(accDist - hitDist) >= 0.0f)
        {
            // m_opaqueList[0]->UpdateWorldMatrix(m_opaqueList[0]->GetWorldRow() *
            //                                    Matrix::CreateTranslation(0.0f, 1.0f, 0.0f));
            std::cout << accDist - hitDist << std::endl;
            m_moveFlag[3] = false;
            prevRayDir    = Vector3(0.0f);
        }
        else
        {
            m_moveFlag[3] = true;
            
        }
    }

    prevPos = curPos;

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
        centerTop    = Vector3::Transform(centerBottom, Matrix::CreateFromQuaternion(q));

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v2 - centerBottom));
        }
        // m_meshes.push_back(cylinder);

        m_cylinderCollider[0].bottomCenter = centerBottom;
        m_cylinderCollider[0].topCenter    = centerTop;
        m_cylinderCollider[0].radius       = radius;

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

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v0 - centerBottom));
        }
        // m_meshes.push_back(cylinder);

        m_cylinderCollider[1].bottomCenter = centerBottom;
        m_cylinderCollider[1].topCenter    = centerTop;
        m_cylinderCollider[1].radius       = radius;

        // cylinder #3
        cylinder     = GeometryGenerator::MakeCylinder(radius, radius, (v0 - v2).Length(), 25);
        p0           = cylinder.vertices[0].position;
        p1           = Vector3::Transform(p0, Matrix::CreateRotationY(XM_PI));
        centerBottom = (p0 + p1) * 0.5f;
        centerTop =
            Vector3::Transform(centerBottom, Matrix::CreateTranslation(Vector3(0.0f, (v0 - v2).Length(), 0.0f)));
        auto q1 = Quaternion::FromToRotation(Vector3(0.0f, 1.0f, 0.0f), v0 - v2);
        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateFromQuaternion(q1));
        }
        centerBottom = Vector3::Transform(centerBottom, Matrix::CreateFromQuaternion(q1));

        for (auto &v : cylinder.vertices)
        {
            v.position = Vector3::Transform(v.position, Matrix::CreateTranslation(v2 - centerBottom));
        }
        // m_meshes.push_back(cylinder);

        m_cylinderCollider[2].bottomCenter = centerBottom;
        m_cylinderCollider[2].topCenter    = centerTop;
        m_cylinderCollider[2].radius       = radius;
    }
}
