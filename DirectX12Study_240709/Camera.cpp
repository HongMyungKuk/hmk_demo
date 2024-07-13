#include "pch.h"

#include "Camera.h"

XMMATRIX Camera::GetViewMatrix()
{
    return XMMatrixTranslation(-m_eyePosition.x, -m_eyePosition.y, -m_eyePosition.z) * XMMatrixRotationY(-m_pitch) *
           XMMatrixRotationX(m_yaw);
}

XMMATRIX Camera::GetProjectionMatrix()
{
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), m_aspect, m_nearZ, m_farZ);
}

void Camera::MouseUpdate(float ndcX, float ndcY)
{
    m_pitch = ndcX * XM_PI;
    m_yaw   = ndcY * XM_PIDIV2;

    XMFLOAT3 directionStart = XMFLOAT3(0.0f, 0.0f, 1.0f);

    auto eyeDirVector = XMVector3Transform(XMLoadFloat3(&directionStart), XMMatrixRotationY(m_pitch));
    XMStoreFloat3(&m_eyeDirection, eyeDirVector);

    XMFLOAT3 upDirectionStart = XMFLOAT3(0.0f, 1.0f, 0.0f);
    auto rightDirVector       = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&upDirectionStart), eyeDirVector));

    XMStoreFloat3(&m_rightDirection, rightDirVector);
}

void Camera::MoveFront(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_eyeDirection) * m_speed * dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}

void Camera::MoveBack(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_eyeDirection) * m_speed * -dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}

void Camera::MoveRight(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_rightDirection) * m_speed * dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}

void Camera::MoveLeft(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_rightDirection) * m_speed * -dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}

void Camera::MoveUp(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_upDirection) * m_speed * dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}

void Camera::MoveDown(const float dt)
{
    auto newPosition = XMVectorAdd(XMLoadFloat3(&m_eyePosition), XMLoadFloat3(&m_upDirection) * m_speed * -dt);
    XMStoreFloat3(&m_eyePosition, newPosition);
}
