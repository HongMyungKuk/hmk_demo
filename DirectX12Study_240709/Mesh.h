#pragma once

#include <directxmath.h>
#include <vector>

using namespace DirectX;

struct Vertex
{
    Vertex() : position(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f)
    {
    }
    Vertex(const XMFLOAT3 &p, const XMFLOAT3 &n) : position(p), normal(n)
    {
    }
    Vertex(float px, float py, float pz, float nx, float ny, float nz) : position(px, py, pz), normal(nx, ny, nz)
    {
    }

    XMFLOAT3 position;
    XMFLOAT3 normal;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};