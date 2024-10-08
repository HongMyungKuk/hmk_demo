#pragma once

#include "AppBase.h"

using namespace DirectX;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

class Camera
{
  public:
    Camera()
    {
        m_startEyePosition = m_eyePosition;
    }

  public:
    Matrix GetViewMatrix();
    Matrix GetProjectionMatrix();
    void MouseUpdate(float ndcX, float ndcY);

    void MoveFront(const float dt);
    void MoveBack(const float dt);
    void MoveRight(const float dt);
    void MoveLeft(const float dt);
    void MoveUp(const float dt);
    void MoveDown(const float dt);
    void Rotation(SimpleMath::Quaternion q);

    Vector3 GetPosition()
    {
        return m_eyePosition;
    }
    Vector3 GetDirection()
    {
        return m_eyeDirection;
    }
    Vector3 GetRightDirection()
    {
        return m_rightDirection;
    }
    void SetDirection(Vector3 v)
    {
        m_eyeDirection = v;
    }
    void SetPosition(Vector3 p)
    {
        m_eyePosition = p;
    }
    const float GetCameraSpeed()
    {
        return m_speed;
    }
    void SetCameraSpeed(const float s)
    {
        m_speed = s;
    }
    float GetNearZ()
    {
        return m_nearZ;
    }
    float GetFarZ()
    {
        return m_farZ;
    }
    float GetPitch()
    {
        return m_pitch;
    }

  private:
    Vector3 m_startEyePosition;
    Vector3 m_eyePosition    = Vector3(0.0f, 3.0f, -5.0f);
    Vector3 m_eyeDirection   = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_upDirection    = Vector3(0.0f, 1.0f, 0.0f);
    Vector3 m_rightDirection = Vector3(1.0f, 0.0f, 0.0f);

    float m_yaw   = 0.0f; //-0.785398f;
    float m_pitch = 0.0f; //-0.523598f * 0.5f;
    float m_roll  = 0.0f;

    SimpleMath::Quaternion m_q = SimpleMath::Quaternion();

    float m_fov   = 70.0f;
    float m_nearZ = 0.1f;
    float m_farZ  = 500.0f;

    float m_speed = 1.0f;

    Matrix m_view;
};
