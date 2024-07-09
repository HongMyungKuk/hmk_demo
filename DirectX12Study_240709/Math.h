#pragma once

namespace DirectX
{
namespace Math
{
struct Vector2;
struct Vector3;
struct Vector4;

//  Vector2
struct Vector2 : public XMFLOAT2
{
    Vector2() : XMFLOAT2(0.0f, 0.0f)
    {
    }
    explicit Vector2(float ix) : XMFLOAT2(ix, ix)
    {
    }
    Vector2(float ix, float iy) : XMFLOAT2(ix, iy)
    {
    }

    Vector2(const Vector2 &)            = default;
    Vector2 &operator=(const Vector2 &) = default;

    Vector2(Vector2 &&)            = default;
    Vector2 &operator=(Vector2 &&) = default;

    // Comparison operators
    bool operator==(const Vector2 &V) const noexcept;
    bool operator!=(const Vector2 &V) const noexcept;

    // Assignment operators
    Vector2 &operator=(const XMVECTORF32 &F) noexcept
    {
        x = F.f[0];
        y = F.f[1];
        return *this;
    }
    Vector2 &operator+=(const Vector2 &V) noexcept;
    Vector2 &operator-=(const Vector2 &V) noexcept;
    Vector2 &operator*=(const Vector2 &V) noexcept;
    Vector2 &operator*=(float S) noexcept;
    Vector2 &operator/=(float S) noexcept;

    // Unary operators
    Vector2 operator+() const noexcept
    {
        return *this;
    }
    Vector2 operator-() const noexcept
    {
        return Vector2(-x, -y);
    }
};
// Binary operators
Vector2 operator+(const Vector2 &V1, const Vector2 &V2) noexcept;
Vector2 operator-(const Vector2 &V1, const Vector2 &V2) noexcept;
Vector2 operator*(const Vector2 &V1, const Vector2 &V2) noexcept;
Vector2 operator*(const Vector2 &V, float S) noexcept;
Vector2 operator/(const Vector2 &V1, const Vector2 &V2) noexcept;
Vector2 operator/(const Vector2 &V, float S) noexcept;
Vector2 operator*(float S, const Vector2 &V) noexcept;

// Vector3
struct Vector3 : public XMFLOAT3
{
    Vector3() : XMFLOAT3(0.0f, 0.0f, 0.0f)
    {
    }
    explicit Vector3(float ix) : XMFLOAT3(ix, ix, ix)
    {
    }
    Vector3(float ix, float iy, float iz) : XMFLOAT3(ix, iy, iz)
    {
    }

    Vector3(const Vector3 &)            = default;
    Vector3 &operator=(const Vector3 &) = default;

    Vector3(Vector3 &&)            = default;
    Vector3 &operator=(Vector3 &&) = default;

    // Comparison operators
    bool operator==(const Vector3 &V) const noexcept;
    bool operator!=(const Vector3 &V) const noexcept;

    // Assignment operators
    Vector3 &operator=(const XMVECTORF32 &F) noexcept
    {
        x = F.f[0];
        y = F.f[1];
        z = F.f[2];

        return *this;
    }
    Vector3 &operator+=(const Vector3 &V) noexcept;
    Vector3 &operator-=(const Vector3 &V) noexcept;
    Vector3 &operator*=(const Vector3 &V) noexcept;
    Vector3 &operator*=(float S) noexcept;
    Vector3 &operator/=(float S) noexcept;

    // Unary operators
    Vector3 operator+() const noexcept
    {
        return *this;
    }
    Vector3 operator-() const noexcept
    {
        return Vector3(-x, -y, -z);
    }
};
// Binary operators
Vector3 operator+(const Vector3 &V1, const Vector3 &V2) noexcept;
Vector3 operator-(const Vector3 &V1, const Vector3 &V2) noexcept;
Vector3 operator*(const Vector3 &V1, const Vector3 &V2) noexcept;
Vector3 operator*(const Vector3 &V, float S) noexcept;
Vector3 operator/(const Vector3 &V1, const Vector3 &V2) noexcept;
Vector3 operator/(const Vector3 &V, float S) noexcept;
Vector3 operator*(float S, const Vector3 &V) noexcept;

} // namespace Math
} // namespace DirectX