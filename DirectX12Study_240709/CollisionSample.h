#pragma once

#include "AppBase.h"

class CollisionSample : public AppBase
{
    struct SphereCollider
    {
        float radius;
        Vector3 center = Vector3(0.0f);
    };

    struct PlaneCollider
    {
        Vector3 normal;
        Vector3 center = Vector3(0.0f);
    };

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
    Vector3 m_collisionColor = Vector3(0.2f, 0.6f, 0.2f);

    // obj1 의 구 충돌체.
    SphereCollider m_sphereCollider;
    PlaneCollider m_planeCollider;

    bool m_moveFlag[4] = {true,true,true,true};
};
