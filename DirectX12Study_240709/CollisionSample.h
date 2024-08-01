#pragma once

#include "AppBase.h"

struct Ray
{
    Ray() : direction(Vector3(0.0f, 0.0f, -1.0f)), startPosition(Vector3(0.0f))
    {
    }

    Ray(Vector3 _direction, Vector3 _startPosition) : direction(_direction), startPosition(_startPosition)
    {
    }

    Vector3 direction;
    Vector3 startPosition;
};

struct SphereCollider
{
    float radius;
    Vector3 center = Vector3(0.0f);

    bool CheckRayToSphereIntersect(Ray ray, Vector3 &closestHit)
    {
        Vector3 d = ray.direction;
        Vector3 o = ray.startPosition;

        float b = o.Dot(d) - center.Dot(d);
        float c = (o - center).Dot(o - center) - radius * radius;

        float det = b * b - c;

        float t = 0.0f;
        if (det >= 0)
        {
            float t1 = -b + sqrtf(det);
            float t2 = -b + sqrtf(det);

            if ((t1 * d).Length() > (t2 * d).Length())
            {
                t = t2;
            }
            else
            {
                t = t1;
            }

            closestHit = o + d * t;

            m_collisionFlag = true;

            return true;
        }
        return false;
    }

    bool m_collisionFlag = false;
};

struct PlaneCollider
{
    Vector3 normal;
    Vector3 center = Vector3(0.0f);
};

struct TriangleCollider
{
    Vector3 normal; // 노말 필요 없음... 편의 상 설정.
    std::vector<Vector3> positions;

    bool CheckRayToTriangleIntersect(Ray ray, Vector3 &closestHit, float& hitDistance)
    {
        Vector3 d = ray.direction;
        Vector3 o = ray.startPosition;

        Vector3 v0 = positions[0];
        Vector3 v1 = positions[1];
        Vector3 v2 = positions[2];

        float numer = v0.Dot(normal) - o.Dot(normal);
        float denom = normal.Dot(d);

        float t = numer / denom;

        closestHit = o + t * d; // closest hit position.
        hitDistance = (t * d).Length();

        Vector3 a1 = (closestHit - v0).Cross(v2 - v0);
        Vector3 a2 = (closestHit - v1).Cross(v0 - v1);
        Vector3 a3 = (closestHit - v2).Cross(v1 - v2);
        a1.Normalize();
        a2.Normalize();
        a3.Normalize();

        if (a1.Length() <= 1e-5 || a2.Length() <= 1e-5 || a3.Length() <= 1e-5)
        {
            return false;
        }

        float alpha1 = a1.Dot(normal);
        float alpha2 = a2.Dot(normal);
        float alpha3 = a3.Dot(normal);

        if (alpha1 >= 0.0f && alpha2 >= 0.0f && alpha3 >= 0.0f)
        {
            m_collisionFlag = true;

            return true;
        }

        return false;
    }

    bool m_collisionFlag = false;
};

struct CylinderCollider
{
    Vector3 topCenter;
    Vector3 bottomCenter;
    Vector3 dirVector;
    float radius;
    float height;
};

class TransformTriangleColiider
{
  public:
    void Collider()
    {
    }

    void Initialize(const float radius, const Vector3 v0, const Vector3 v1, const Vector3 v2);

    bool CheckRayIntersect(Ray ray)
    {
        // 총 7개의 기하구조와 충돌을 체크.
        float dist = D3D12_FLOAT32_MAX;

        // 면에 충돌했을때 == 삼각형에 충돌
        if (m_triangleCollider.CheckRayToTriangleIntersect(ray, hitPostion, hitDistance))
        {
            return true;
        }

        // 모서리에 충돌했을때 == 실린더에 충돌

        // 꼭지점에 충돌했을때 == 구에 충돌
        if (m_sphereCollider[0].CheckRayToSphereIntersect(ray, hitPostion))
        {
            return true;
        }

        if (m_sphereCollider[1].CheckRayToSphereIntersect(ray, hitPostion))
        {
            return true;
        }

        if (m_sphereCollider[2].CheckRayToSphereIntersect(ray, hitPostion))
        {
            return true;
        }

        return false;
    }

    void BehaviorAfterCollsion()
    {
        if (m_triangleCollider.m_collisionFlag)
        {
            m_color                            = m_collisionColor;
            m_triangleCollider.m_collisionFlag = false;
        }

        if (m_sphereCollider[0].m_collisionFlag)
        {
            m_color                             = m_collisionColor;
            m_sphereCollider[0].m_collisionFlag = false;
        }
        if (m_sphereCollider[1].m_collisionFlag)
        {
            m_color                             = m_collisionColor;
            m_sphereCollider[1].m_collisionFlag = false;
        }
        if (m_sphereCollider[2].m_collisionFlag)
        {
            m_color                             = m_collisionColor;
            m_sphereCollider[2].m_collisionFlag = false;
        }
    }

    std::vector<MeshData> GetMeshes()
    {
        return m_meshes;
    }

  public:
    SphereCollider m_sphereCollider[3];
    TriangleCollider m_triangleCollider;
    CylinderCollider m_cylinderCollider[3];

    Vector3 m_color          = Vector3(0.2f, 0.2f, 0.6f);
    Vector3 m_collisionColor = Vector3(0.2f, 0.6f, 0.2f);

    float m_radius;
    std::vector<MeshData> m_meshes;

    Vector3 hitPostion = Vector3(0.0f);
    float hitDistance  = 0.0f;
};

class CollisionSample : public AppBase
{
    virtual bool Initialize();
    virtual void UpdateGui(const float frameRate);
    virtual void Render();
    virtual void Update(const float dt);

    virtual void UpdateLights();

    // light on/off flag
    bool m_useDL = true;  // directional light
    bool m_usePL = false; // point lihgt
    bool m_useSL = false; // spot light

    Vector3 m_obj1Color      = Vector3(0.6f, 0.2f, 0.2f);
    Vector3 m_obj2Color      = Vector3(0.2f, 0.2f, 0.6f);
    Vector3 m_obj3Color      = Vector3(0.6f, 0.2f, 0.6f);
    Vector3 m_collisionColor = Vector3(0.2f, 0.6f, 0.2f);

    // obj1 의 구 충돌체.
    SphereCollider m_sphereCollider;
    PlaneCollider m_planeCollider;
    TriangleCollider m_triangleCollider;
    CylinderCollider m_cylinderCollider;

    bool m_moveFlag[4] = {true, true, true, true};

    bool m_collisionFlag[3] = {false, false, false};

    std::map<std::string, float> m_distList;

    // Collider 생성.
    TransformTriangleColiider m_collider;
};
