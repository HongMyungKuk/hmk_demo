#pragma once

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

  private:
    XMFLOAT3 m_eyePosition    = XMFLOAT3(0.0f, 3.0f, -3.0f);
    XMFLOAT3 m_eyeDirection   = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 m_upDirection    = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 m_rightDirection = XMFLOAT3(1.0f, 0.0f, 0.0f);

    float m_yaw   = 0.0f;
    float m_pitch = 0.0f;
    float m_roll  = 0.0f;

    float m_fov = 70.0f;
};
