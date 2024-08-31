#include "pch.h"

#include "Camera.h"

//// First person view caemra.
//Matrix Camera::GetViewMatrix()
//{
//    return Matrix::CreateTranslation(-m_eyePosition) * Matrix::CreateRotationY(-m_pitch) *
//           Matrix::CreateRotationX(m_yaw) * Matrix::CreateFromQuaternion(m_q);
//}

Matrix Camera::GetViewMatrix()
{
    m_view = Matrix::CreateTranslation(m_startEyePosition - m_eyePosition) * Matrix::CreateRotationY(-m_pitch) * Matrix::CreateRotationX(m_yaw) * Matrix::CreateFromQuaternion(m_q) *
             Matrix::CreateTranslation(-m_eyePosition) * Matrix::CreateTranslation(m_eyePosition - m_startEyePosition);

    return m_view;
}

Matrix Camera::GetProjectionMatrix()
{
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), AppBase::GetAspect(), m_nearZ, m_farZ);
}

void Camera::MouseUpdate(float ndcX, float ndcY)
{
    m_pitch = ndcX * XM_PI;
    m_yaw   = ndcY * XM_PIDIV2;

    m_eyeDirection   = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateRotationY(m_pitch));
    m_rightDirection = m_upDirection.Cross(m_eyeDirection);
    m_rightDirection.Normalize();
}

void Camera::MoveFront(const float dt)
{
    m_eyePosition += m_eyeDirection * m_speed * dt;
}

void Camera::MoveBack(const float dt)
{
    m_eyePosition += m_eyeDirection * m_speed * -dt;
}

void Camera::MoveRight(const float dt)
{
    m_eyePosition += m_rightDirection * m_speed * dt;
}

void Camera::MoveLeft(const float dt)
{
    m_eyePosition += m_rightDirection * m_speed * -dt;
}

void Camera::MoveUp(const float dt)
{
    m_eyePosition += m_upDirection * m_speed * dt;
}

void Camera::MoveDown(const float dt)
{
    m_eyePosition += m_upDirection * m_speed * -dt;
}

void Camera::Rotation(SimpleMath::Quaternion q)
{
    m_q = q;
}
