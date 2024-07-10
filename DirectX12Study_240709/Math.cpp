#include "pch.h"

#include "Math.h"

namespace DirectX
{
namespace Math
{
bool Vector2::operator==(const Vector2 &V) const noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR v2 = XMLoadFloat2(&V);
    return XMVector2Equal(v1, v2);
}

bool Vector2::operator!=(const Vector2 &V) const noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR v2 = XMLoadFloat2(&V);
    return XMVector2NotEqual(v1, v2);
}
Vector2 &Vector2::operator+=(const Vector2 &V) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR v2 = XMLoadFloat2(&V);
    const XMVECTOR r  = XMVectorAdd(v1, v2);
    XMStoreFloat2(this, r);
    return *this;
}
Vector2 &Vector2::operator-=(const Vector2 &V) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR v2 = XMLoadFloat2(&V);
    const XMVECTOR r  = XMVectorSubtract(v1, v2);
    XMStoreFloat2(this, r);
    return *this;
}
Vector2 &Vector2::operator*=(const Vector2 &V) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR v2 = XMLoadFloat2(&V);
    const XMVECTOR r  = XMVectorMultiply(v1, v2);
    XMStoreFloat2(this, r);
    return *this;
}
Vector2 &Vector2::operator*=(float S) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR r  = XMVectorScale(v1, S);
    XMStoreFloat2(this, r);
    return *this;
}
Vector2 &Vector2::operator/=(float S) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(this);
    const XMVECTOR r  = XMVectorScale(v1, 1.0f / S);
    XMStoreFloat2(this, r);
    return *this;
}
Vector2 operator+(const Vector2 &V1, const Vector2 &V2) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(&V1);
    const XMVECTOR v2 = XMLoadFloat2(&V2);
    const XMVECTOR r  = XMVectorAdd(v1, v2);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator-(const Vector2 &V1, const Vector2 &V2) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(&V1);
    const XMVECTOR v2 = XMLoadFloat2(&V2);
    const XMVECTOR r  = XMVectorSubtract(v1, v2);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator*(const Vector2 &V1, const Vector2 &V2) noexcept
{
    const XMVECTOR v1 = XMLoadFloat2(&V1);
    const XMVECTOR v2 = XMLoadFloat2(&V2);
    const XMVECTOR r  = XMVectorMultiply(v1, v2);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator*(const Vector2 &V, float S) noexcept
{
    const XMVECTOR v1  = XMLoadFloat2(&V);
    const XMVECTOR r = XMVectorScale(v1, S);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator/(const Vector2 &V1, const Vector2 &V2) noexcept
{
    const XMVECTOR v1  = XMLoadFloat2(&V1);
    const XMVECTOR v2  = XMLoadFloat2(&V2);
    const XMVECTOR r = XMVectorDivide(v1, v2);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator/(const Vector2 &V, float S) noexcept
{
    const XMVECTOR v1  = XMLoadFloat2(&V);
    const XMVECTOR r = XMVectorScale(v1, 1.0f/S);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
Vector2 operator*(float S, const Vector2 &V) noexcept
{
    const XMVECTOR v1  = XMLoadFloat2(&V);
    const XMVECTOR r = XMVectorScale(v1, S);
    Vector2 ans;
    XMStoreFloat2(&ans, r);
    return ans;
}
bool Vector3::operator==(const Vector3 &V) const noexcept
{
    return false;
}
bool Vector3::operator!=(const Vector3 &V) const noexcept
{
    return false;
}
Vector3 &Vector3::operator+=(const Vector3 &V) noexcept
{
    // TODO: insert return statement here
    return *this;
}
Vector3 &Vector3::operator-=(const Vector3 &V) noexcept
{
    // TODO: insert return statement here
    return *this;
}
Vector3 &Vector3::operator*=(const Vector3 &V) noexcept
{
    // TODO: insert return statement here
    return *this;
}
Vector3 &Vector3::operator*=(float S) noexcept
{
    // TODO: insert return statement here
    return *this;
}
Vector3 &Vector3::operator/=(float S) noexcept
{
    // TODO: insert return statement here
    return *this;
}
Vector3 operator+(const Vector3 &V1, const Vector3 &V2) noexcept
{
    return Vector3();
}
Vector3 operator-(const Vector3 &V1, const Vector3 &V2) noexcept
{
    return Vector3();
}
Vector3 operator*(const Vector3 &V1, const Vector3 &V2) noexcept
{
    return Vector3();
}
Vector3 operator*(const Vector3 &V, float S) noexcept
{
    return Vector3();
}
Vector3 operator/(const Vector3 &V1, const Vector3 &V2) noexcept
{
    return Vector3();
}
Vector3 operator/(const Vector3 &V, float S) noexcept
{
    return Vector3();
}
Vector3 operator*(float S, const Vector3 &V) noexcept
{
    return Vector3();
}
} // namespace Math
} // namespace DirectX