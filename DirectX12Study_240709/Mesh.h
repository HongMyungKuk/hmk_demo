#pragma once

#include "DescriptorHeap.h"

using namespace DirectX;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

struct Vertex
{
    Vertex() : position(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), texCoord(0.0f, 0.0f)
    {
    }
    Vertex(const Vector3 &p, const Vector3 &n, const Vector2 &t) : position(p), normal(n), texCoord(t)
    {
    }
    Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty)
        : position(px, py, pz), normal(nx, ny, nz), texCoord(tx, ty)
    {
    }

    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    Vector3 tangent = Vector3(1.0f, 0.0f, 0.0f);
};

struct SkinnedVertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    Vector3 tangent;
    float boneWeights[8]   = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    uint8_t boneIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0};
};

struct MeshData
{
    using index_t = uint32_t;

    std::vector<Vertex> vertices;
    std::vector<SkinnedVertex> skinnedVertices;
    std::vector<index_t> indices;

    std::string albedoTextureFilename    = "";
    std::string metallicTextureFilename  = "";
    std::string roughnessTextureFilename = "";
    std::string normalTextureFilename    = "";
    std::string heightTextureFilename    = "";
    std::string aoTextureFilename        = "";
    std::string emissionTextureFilename  = "";
};

struct Mesh
{
    ID3D12Resource *vertexBuffer = nullptr;
    ID3D12Resource *indexBuffer  = nullptr;
    uint32_t vertexCount         = 0;
    uint32_t indexCount          = 0;
    uint32_t stride              = 0;
    // Teture
    ID3D12Resource *albedoTexture       = nullptr;
    ID3D12Resource *albedoUploadTexture = nullptr;
    DescriptorHandle albedoDescriptorHandle;
    ID3D12Resource *metallicTexture       = nullptr;
    ID3D12Resource *metallicUploadTexture = nullptr;
    DescriptorHandle metallicDescriptorHandle;
    ID3D12Resource *roughnessTexture     = nullptr;
    ID3D12Resource *roughnessloadTexture = nullptr;
    DescriptorHandle roughnessDescriptorHandle;
    ID3D12Resource *normalTexture     = nullptr;
    ID3D12Resource *normalLoadTexture = nullptr;
    DescriptorHandle normalDescriptorHandle;
    ID3D12Resource *heightTexture     = nullptr;
    ID3D12Resource *heightLoadTexture = nullptr;
    DescriptorHandle heightDescriptorHandle;
    ID3D12Resource *aoTexture     = nullptr;
    ID3D12Resource *aoLoadTexture = nullptr;
    DescriptorHandle aoDescriptorHandle;
    ID3D12Resource *emissionTexture     = nullptr;
    ID3D12Resource *emissionLoadTexture = nullptr;
    DescriptorHandle emissionDescriptorHandle;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView()
    {
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        view.StrideInBytes  = stride;
        view.SizeInBytes    = vertexCount * view.StrideInBytes;
        return view;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView()
    {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        view.SizeInBytes    = indexCount * sizeof(MeshData::index_t);
        view.Format         = DXGI_FORMAT_R32_UINT;
        return view;
    }
};