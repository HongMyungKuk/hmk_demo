#pragma once

#include "AppBase.h"

using namespace DirectX;

class Camera
{
  public:
    XMFLOAT3 GetPosition()
    {
        return m_eyePosition;
    }
    void SetPosition(XMFLOAT3 p)
    {
        m_eyePosition = p;
    }
    XMMATRIX GetViewMatrix();
    XMMATRIX GetProjectionMatrix();
    void MouseUpdate(float ndcX, float ndcY);
    
    void MoveFront(const float dt);
    void MoveBack(const float dt);
    void MoveRight(const float dt);
    void MoveLeft(const float dt);
    void MoveUp(const float dt);
    void MoveDown(const float dt);

  private:
    XMFLOAT3 m_eyePosition    = XMFLOAT3(1.0f, 3.0f, -3.0f);
    XMFLOAT3 m_eyeDirection   = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 m_upDirection    = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 m_rightDirection = XMFLOAT3(1.0f, 0.0f, 0.0f);

    float m_yaw   = -0.785398f;
    float m_pitch = -0.523598f * 0.5f;
    float m_roll  = 0.0f;

    float m_fov = 70.0f;
    float m_aspect = AppBase::GetAspect();
    float m_nearZ  = 0.1f;
    float m_farZ   = 100.0f;
};
