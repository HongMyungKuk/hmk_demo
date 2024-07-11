#include "pch.h"

#include "Camera.h"

XMMATRIX Camera::GetViewMatrix()
{
    return XMMatrixTranslation(-m_eyePosition.x, -m_eyePosition.y, -m_eyePosition.z) * XMMatrixRotationX(m_pitch) *
           XMMatrixRotationY(m_yaw);
}

XMMATRIX Camera::GetProjectionMatrix()
{
    return XMMATRIX();
}

void Camera::MouseUpdate(float ndcX, float ndcY)
{
}
