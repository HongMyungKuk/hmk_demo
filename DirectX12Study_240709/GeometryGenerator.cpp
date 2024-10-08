#include "pch.h"

#include "GeometryGenerator.h"
#include "ModelLoader.h"
#include <DirectXMesh.h>

MeshData GeometryGenerator::MakeTriangle(const float x)
{
    MeshData meshData = {};

    float w2 = 0.5f * x;
    float h2 = w2 * sqrtf(3.0);

    std::vector<Vertex> &vertices           = meshData.vertices;
    std::vector<MeshData::index_t> &indices = meshData.indices;

    Vertex v;
    v.position = Vector3(-w2, -h2 * 1.0f / 3.0f, 0.0f);
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(0.0f, 1.0f);
    vertices.push_back(v);

    v.position = Vector3(0.0f, h2 * 2.0f / 3.0f, 0.0f);
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(0.5f, 0.0f);
    vertices.push_back(v);

    v.position = Vector3(w2, -h2 * 1.0f / 3.0f, 0.0f);
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(1.0f, 1.0f);
    vertices.push_back(v);

    indices = {0, 1, 2};

    return meshData;
}

MeshData GeometryGenerator::MakeTriangle(const Vector3 v0, const Vector3 v1, const Vector3 v2)
{
    MeshData meshData = {};

    std::vector<Vertex> &vertices           = meshData.vertices;
    std::vector<MeshData::index_t> &indices = meshData.indices;

    Vertex v;
    v.position = v0;
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(0.0f, 1.0f);
    vertices.push_back(v);

    v.position = v1;
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(0.5f, 0.0f);
    vertices.push_back(v);

    v.position = v2;
    v.normal   = Vector3(0.0f, 0.0f, -1.0f);
    v.texCoord = Vector2(1.0f, 1.0f);
    vertices.push_back(v);

    indices = {0, 1, 2};

    return meshData;
}

MeshData GeometryGenerator::MakeSquare(const float w, const float h)
{
    MeshData meshData = {};

    float w2 = 0.5f * w;
    float h2 = 0.5f * h;

    std::vector<Vertex> &vertices           = meshData.vertices;
    std::vector<MeshData::index_t> &indices = meshData.indices;

    vertices = {{Vector3(-w2, -h2, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)},
                {Vector3(-w2, h2, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)},
                {Vector3(w2, h2, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)},
                {Vector3(w2, -h2, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)}};

    indices = {
        0, 1, 3, 1, 2, 3,
    };

    return meshData;
}

MeshData GeometryGenerator::MakeSquareGrid(const int numSlices, const int numStacks, const float scale,
                                           const Vector2 texScale)
{
    MeshData meshData;

    float dx = 2.0f / numSlices;
    float dy = 2.0f / numStacks;

    float y = 1.0f;
    for (int j = 0; j < numStacks + 1; j++)
    {
        float x = -1.0f;
        for (int i = 0; i < numSlices + 1; i++)
        {
            Vertex v;
            v.position = Vector3(x, y, 0.0f) * scale;
            v.normal   = Vector3(0.0f, 0.0f, -1.0f);
            v.texCoord = Vector2(x + 1.0f, y + 1.0f) * 0.5f * texScale;
            v.tangent  = Vector3(1.0f, 0.0f, 0.0f);

            meshData.vertices.push_back(v);

            x += dx;
        }
        y -= dy;
    }

    for (int j = 0; j < numStacks; j++)
    {
        for (int i = 0; i < numSlices; i++)
        {
            meshData.indices.push_back((numSlices + 1) * j + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i + 1);
        }
    }

    return meshData;
}

MeshData GeometryGenerator::MakeCylinder(const float bottomRadius, const float topRadius, float height, int numSlices)
{

    // Texture 좌표계때문에 (numSlices + 1) x 2 개의 버텍스 사용

    const float dTheta = -XM_2PI / float(numSlices);

    MeshData meshData;

    std::vector<Vertex> &vertices = meshData.vertices;

    // 옆면의 바닥 버텍스들 (인덱스 0 이상 numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f), Matrix::CreateRotationY(dTheta * float(i)));

        // std::cout << v.position.x << " " << v.position.z << std::endl;

        v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
        v.normal.Normalize();
        v.texCoord = Vector2(float(i) / numSlices, 1.0f);

        vertices.push_back(v);
    }

    // 옆면의 맨 위 버텍스들 (인덱스 numSlices 이상 2 * numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f), Matrix::CreateRotationY(dTheta * float(i)));
        v.normal = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
        v.normal.Normalize();
        v.texCoord = Vector2(float(i) / numSlices, 0.0f);

        vertices.push_back(v);
    }

    std::vector<uint32_t> &indices = meshData.indices;

    for (int i = 0; i < numSlices; i++)
    {
        indices.push_back(i);
        indices.push_back(i + numSlices + 1);
        indices.push_back(i + 1 + numSlices + 1);

        indices.push_back(i);
        indices.push_back(i + 1 + numSlices + 1);
        indices.push_back(i + 1);
    }

    return meshData;
}

MeshData GeometryGenerator::MakeCube(const float w, const float h, const float d)
{
    MeshData meshData;

    Vertex v[24];

    float w2 = 0.5f * w;
    float h2 = 0.5f * h;
    float d2 = 0.5f * d;

    // Fill in the front face vertex data.
    v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // Fill in the back face vertex data.
    v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    // Fill in the top face vertex data.
    v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the bottom face vertex data.
    v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f);
    v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f);
    v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f);

    // Fill in the left face vertex data.
    v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the right face vertex data.
    v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // meshData.vertices.assign(&v[0], &v[24]);
    meshData.vertices.resize(24);
    meshData.vertices.assign(&v[0], &v[24]);

    MeshData::index_t i[36];

    // Fill in the front face index data
    i[0] = 0;
    i[1] = 1;
    i[2] = 2;
    i[3] = 0;
    i[4] = 2;
    i[5] = 3;

    // Fill in the back face index data
    i[6]  = 4;
    i[7]  = 5;
    i[8]  = 6;
    i[9]  = 4;
    i[10] = 6;
    i[11] = 7;

    // Fill in the top face index data
    i[12] = 8;
    i[13] = 9;
    i[14] = 10;
    i[15] = 8;
    i[16] = 10;
    i[17] = 11;

    // Fill in the bottom face index data
    i[18] = 12;
    i[19] = 13;
    i[20] = 14;
    i[21] = 12;
    i[22] = 14;
    i[23] = 15;

    // Fill in the left face index data
    i[24] = 16;
    i[25] = 17;
    i[26] = 18;
    i[27] = 16;
    i[28] = 18;
    i[29] = 19;

    // Fill in the right face index data
    i[30] = 20;
    i[31] = 21;
    i[32] = 22;
    i[33] = 20;
    i[34] = 22;
    i[35] = 23;

    meshData.indices.resize(36);
    meshData.indices.assign(&i[0], &i[36]);

    return meshData;
}

MeshData GeometryGenerator::MakeSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    MeshData meshData;

    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

    meshData.vertices.push_back(topVertex);

    float phiStep   = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    // Compute vertices for each stack ring (do not count the poles as rings).
    for (uint32_t i = 1; i <= stackCount - 1; ++i)
    {
        float phi = i * phiStep;

        // Vertices of ring.
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex v;

            // spherical to cartesian
            v.position.x = radius * sinf(phi) * cosf(theta);
            v.position.y = radius * cosf(phi);
            v.position.z = radius * sinf(phi) * sinf(theta);

            XMVECTOR p = XMLoadFloat3(&v.position);
            XMStoreFloat3(&v.normal, XMVector3Normalize(p));

            v.texCoord.x = theta / XM_2PI;
            v.texCoord.y = phi / XM_PI;

            meshData.vertices.push_back(v);
        }
    }

    meshData.vertices.push_back(bottomVertex);

    //
    // Compute indices for top stack.  The top stack was written first to the vertex buffer
    // and connects the top pole to the first ring.
    //

    for (uint32_t i = 1; i <= sliceCount; ++i)
    {
        meshData.indices.push_back(0);
        meshData.indices.push_back(i + 1);
        meshData.indices.push_back(i);
    }

    //
    // Compute indices for inner stacks (not connected to poles).
    //

    // Offset the indices to the index of the first vertex in the first ring.
    // This is just skipping the top pole vertex.
    uint32_t baseIndex       = 1;
    uint32_t ringVertexCount = sliceCount + 1;
    for (uint32_t i = 0; i < stackCount - 2; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    //
    // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
    // and connects the bottom pole to the bottom ring.
    //

    // South pole vertex was added last.
    uint32_t southPoleIndex = (uint32_t)meshData.vertices.size() - 1;

    // Offset the indices to the index of the first vertex in the last ring.
    baseIndex = southPoleIndex - ringVertexCount;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(southPoleIndex);
        meshData.indices.push_back(baseIndex + i);
        meshData.indices.push_back(baseIndex + i + 1);
    }

    std::vector<XMFLOAT3> positions(meshData.vertices.size());
    std::vector<XMFLOAT3> normals(meshData.vertices.size());
    std::vector<XMFLOAT2> texCoords(meshData.vertices.size());
    std::vector<XMFLOAT3> tangents(meshData.vertices.size());
    std::vector<XMFLOAT3> biTangents(meshData.vertices.size());

    for (size_t i = 0; i < meshData.vertices.size(); i++)
    {
        positions[i] = meshData.vertices[i].position;
        normals[i]   = meshData.vertices[i].normal;
        texCoords[i] = meshData.vertices[i].texCoord;
    }

    DirectX::ComputeTangentFrame(meshData.indices.data(), meshData.indices.size() / 3, positions.data(), normals.data(),
                                 texCoords.data(), positions.size(), tangents.data(), biTangents.data());

    for (size_t i = 0; i < tangents.size(); i++)
    {
        meshData.vertices[i].tangent = tangents[i];
    }

    return meshData;
}

void NomalizeModel(std::vector<MeshData> &meshes, const float sacle, AnimationData &aniData)
{
    Vector3 max = Vector3(-1000.0f, -1000.0f, -1000.0f);
    Vector3 min = Vector3(1000.0f, 1000.0f, 1000.0f);

    for (const auto &m : meshes)
    {
        for (const auto &p : m.vertices)
        {
            max.x = DirectX::XMMax(p.position.x, max.x);
            min.x = DirectX::XMMin(p.position.x, min.x);
            max.y = DirectX::XMMax(p.position.y, max.y);
            min.y = DirectX::XMMin(p.position.y, min.y);
            max.z = DirectX::XMMax(p.position.z, max.z);
            min.z = DirectX::XMMin(p.position.z, min.z);
        }
    }

    const float dx = max.x - min.x;
    const float dy = max.y - min.y;
    const float dz = max.z - min.z;

    const float scale   = sacle / DirectX::XMMax(XMMax(dx, dy), dz);
    Vector3 translation = -(max + min) * 0.5f;

    translation.y += dy * 0.5f;

    for (auto &m : meshes)
    {
        for (auto &p : m.vertices)
        {
            p.position = (p.position + translation) * scale;
        }

        for (auto &p : m.skinnedVertices)
        {
            p.position = (p.position + translation) * scale;
        }
    }

    aniData.defaultMatrix = Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
}

auto GeometryGenerator::ReadFromModelFile(const char *filepath, const char *filename, bool isAnim)
    -> std::pair<std::vector<MeshData>, std::vector<MaterialConsts>>
{
    ModelLoader modelLoader((const char *)filepath, (const char *)filename, isAnim);

    auto meshes   = modelLoader.Meshes();
    auto material = modelLoader.Materials();

    NomalizeModel(meshes, 1.0f, modelLoader.Animation());

    // Mesh Data 가 들어있는 vector를 vector 1개의 MeshData로 변환한다.
    return {meshes, material};
}

auto GeometryGenerator::ReadFromAnimationFile(const char *filepath, const char *filename)
    -> std::pair<std::vector<MeshData>, AnimationData>
{
    ModelLoader modelLoader((const char *)filepath, (const char *)filename, true);

    auto meshes = modelLoader.Meshes();
    auto &anim  = modelLoader.Animation();

    NomalizeModel(meshes, 1.0f, anim);

    return {meshes, anim};
}