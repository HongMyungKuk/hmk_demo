#include "pch.h"

#include "Frustum.h"
#include "Model.h"
#include "QuadTree.h"

#include <thread>

void QuadTree::Initialize(Model *terrain, std::vector<MeshData> meshes, ID3D12Device *device,
                          ID3D12GraphicsCommandList *commandList)
{
    float centerX = 0.0f;
    float centerZ = 0.0f;
    float width   = 0.0f;

    m_meshDatas = meshes;

    CaculateMeshDimesion(centerX, centerZ, width);

    m_rootNode = new NodeType;

    CreateTreeNode(m_rootNode, centerX, centerZ, width, device, commandList);
}

void QuadTree::Update()
{
    UpdateNode(m_rootNode);
}

void QuadTree::Render(Frustum *frustum, ID3D12GraphicsCommandList *commandList)
{
    m_drawCount = 0;

    RenderNode(frustum, m_rootNode, commandList);

    // std::cout << m_drawCount << std::endl;
}

void QuadTree::GetHeight(float positionX, float positionZ, float &height)
{
    float minX = m_rootNode->positionX - m_rootNode->width / 2.0f;
    float maxX = m_rootNode->positionX + m_rootNode->width / 2.0f;
    float minZ = m_rootNode->positionZ - m_rootNode->width / 2.0f;
    float maxZ = m_rootNode->positionZ + m_rootNode->width / 2.0f;

    if (positionX < minX || positionX > maxX || positionZ < minZ || positionZ > maxZ)
    {
        return;
    }

    FindNode(m_rootNode, positionX, positionZ, height);
}

void QuadTree::CaculateMeshDimesion(float &centerX, float &centerZ, float &width)
{
    centerX = 0.0f;
    centerZ = 0.0f;

    Vector3 minPos(1000.0f, 1000.0f, 1000.0f);
    Vector3 maxPos(-1000.0f, -1000.0f, -1000.0f);
    for (auto m : m_meshDatas)
    {
        for (auto v : m.vertices)
        {
            minPos.x = DirectX::XMMin(minPos.x, v.position.x);
            minPos.y = DirectX::XMMin(minPos.y, v.position.y);
            minPos.z = DirectX::XMMin(minPos.z, v.position.z);
            maxPos.x = DirectX::XMMax(maxPos.x, v.position.x);
            maxPos.y = DirectX::XMMax(maxPos.y, v.position.y);
            maxPos.z = DirectX::XMMax(maxPos.z, v.position.z);
        }
    }

    centerX = (minPos.x + maxPos.x) * 0.5f;
    centerZ = (minPos.z + maxPos.z) * 0.5f;

    float widthX = maxPos.x - minPos.x;
    float widthZ = maxPos.z - minPos.z;

    width = DirectX::XMMax(widthX, widthZ);
}

void QuadTree::CreateTreeNode(NodeType *node, float positionX, float positionZ, float width, ID3D12Device *device,
                              ID3D12GraphicsCommandList *commandList)
{
    static int nodeIdx = 0;

    node->positionX = positionX;
    node->positionZ = positionZ;
    node->width     = width;

    node->triangleCount = 0;

    node->nodes[0] = nullptr;
    node->nodes[1] = nullptr;
    node->nodes[2] = nullptr;
    node->nodes[3] = nullptr;

    int numTriangles = CountTriangles(node->positionX, node->positionZ, node->width);

    if (numTriangles == 0)
    {
        return;
    }

    if (numTriangles > 2500)
    {
        for (int i = 0; i < 4; i++)
        {
            float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (node->width / 4.0f);
            float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (node->width / 4.0f);

            int count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));

            if (count > 0)
            {
                node->nodes[i] = new NodeType;

                CreateTreeNode(node->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device,
                               commandList);
            }
        }
        return;
    }

    node->triangleCount = numTriangles;

    std::cout << nodeIdx << ": " << numTriangles << std::endl;

    nodeIdx++;

    std::vector<MeshData> meshes;

    std::thread t[8];
    std::vector<MeshData> meshDatas(8);
    for (auto m : m_meshDatas)
    {
        uint32_t indexData = 0;

        int i0;
        int i1;
        int i2;

        bool flag[8] = {};

        t[0] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[0], 0,
                           m.indices.size() / 8, nullptr, &meshDatas[0]);
        t[1] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[1],
                           m.indices.size() / 8, m.indices.size() / 8 * 2, nullptr, &meshDatas[1]);
        t[2] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[2],
                           m.indices.size() / 8 * 2, m.indices.size() / 8 * 3, nullptr, &meshDatas[2]);
        t[3] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[3],
                           m.indices.size() / 8 * 3, m.indices.size() / 8 * 4, nullptr, &meshDatas[3]);
        t[4] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[4],
                           m.indices.size() / 8 * 4, m.indices.size() / 8 * 5, nullptr, &meshDatas[4]);
        t[5] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[5],
                           m.indices.size() / 8 * 5, m.indices.size() / 8 * 6, nullptr, &meshDatas[5]);
        t[6] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[6],
                           m.indices.size() / 8 * 6, m.indices.size() / 8 * 7, nullptr, &meshDatas[6]);
        t[7] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, &flag[7],
                           m.indices.size() / 8 * 7, m.indices.size(), nullptr, &meshDatas[7]);
    }

    t[0].join();
    t[1].join();
    t[2].join();
    t[3].join();
    t[4].join();
    t[5].join();
    t[6].join();
    t[7].join();

    if (meshes.empty())
    {
        meshes.push_back(meshDatas[0]);
    }

    for (auto m : meshDatas)
    {
        for (auto v : m.vertices)
        {
            meshes[0].vertices.push_back(v);
        }
    }

    int indexCount = 0;
    for (auto &m : meshes)
    {
        for (auto &v : m.vertices)
        {
            m.indices.push_back(indexCount++);
        }
    }

    node->model = new Model;
    node->model->Initialize(device, commandList, meshes);
    node->meshData = meshes[0];
}

int QuadTree::CountTriangles(float positionX, float positionZ, float width)
{
    std::thread t[8];

    int sum      = 0;
    int count[8] = {};
    bool flag[8] = {};

    for (auto m : m_meshDatas)
    {
        t[0] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr, 0,
                           m.indices.size() / 8, &count[0], nullptr);

        t[1] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8, m.indices.size() / 8 * 2, &count[1], nullptr);

        t[2] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 2, m.indices.size() / 8 * 3, &count[2], nullptr);

        t[3] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 3, m.indices.size() / 8 * 4, &count[3], nullptr);

        t[4] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 4, m.indices.size() / 8 * 5, &count[4], nullptr);

        t[5] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 5, m.indices.size() / 8 * 6, &count[5], nullptr);

        t[6] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 6, m.indices.size() / 8 * 7, &count[6], nullptr);

        t[7] = std::thread(&QuadTree::IsTriangleContained, this, 0, positionX, positionZ, width, m, nullptr,
                           m.indices.size() / 8 * 7, m.indices.size(), &count[7], nullptr);
    }

    t[0].join();
    t[1].join();
    t[2].join();
    t[3].join();
    t[4].join();
    t[5].join();
    t[6].join();
    t[7].join();

    for (int i = 0; i < 8; i++)
    {
        sum += count[i];
    }

    return sum;
}

void QuadTree::IsTriangleContained(int idx, float positionX, float positionZ, float width, MeshData meshData, bool *b,
                                   int start, int end, int *count, MeshData *newMeshData)
{
    using namespace DirectX;

    for (int i = start; i < end; i += 3)
    {
        float radius = width / 2.0f;

        auto i0 = meshData.indices[i];
        auto i1 = meshData.indices[i + 1];
        auto i2 = meshData.indices[i + 2];

        float x1 = meshData.vertices[i0].position.x;
        float z1 = meshData.vertices[i0].position.z;

        float x2 = meshData.vertices[i1].position.x;
        float z2 = meshData.vertices[i1].position.z;

        float x3 = meshData.vertices[i2].position.x;
        float z3 = meshData.vertices[i2].position.z;

        float minX = XMMin(x1, XMMin(x2, x3));
        if (minX > (positionX + radius))
        {
            if (b != nullptr)
                *b = false;
            continue;
        }

        float maxX = XMMax(x1, XMMax(x2, x3));
        if (maxX < (positionX - radius))
        {
            if (b != nullptr)
                *b = false;
            continue;
        }

        float minZ = XMMin(z1, XMMin(z2, z3));
        if (minZ > (positionZ + radius))
        {
            if (b != nullptr)
                *b = false;
            continue;
        }

        float maxZ = XMMax(z1, XMMax(z2, z3));
        if (maxZ < (positionZ - radius))
        {
            if (b != nullptr)
                *b = false;
            continue;
        }

        if (count != nullptr)
            *count += 1;

        if (newMeshData)
        {
            newMeshData->vertices.push_back(meshData.vertices[i0]);
            newMeshData->vertices.push_back(meshData.vertices[i1]);
            newMeshData->vertices.push_back(meshData.vertices[i2]);
        }
    }

    if (b != nullptr)
    {
        *b = true;
    }
}

void QuadTree::UpdateNode(NodeType *node)
{
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i] != nullptr)
        {
            count++;
            UpdateNode(node->nodes[i]);
        }
    }

    //  이미 자식 노드에서 rendering 을 했기때문에 또 그릴 필요가 없음.
    if (count != 0)
    {
        return;
    }

    node->model->Update();
}

void QuadTree::RenderNode(Frustum *frustum, NodeType *node, ID3D12GraphicsCommandList *commandList)
{
    // node 가 frustum 에 포함되지 않으면 render 하지 않는다.
    if (!frustum->CheckCube(node->positionX, 0.0f, node->positionZ, node->width / 2.0f))
    {
        return;
    }

    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i] != nullptr)
        {
            count++;
            RenderNode(frustum, node->nodes[i], commandList);
        }
    }

    //  이미 자식 노드에서 rendering 을 했기때문에 또 그릴 필요가 없음.
    if (count != 0)
    {
        return;
    }

    node->model->Render(commandList);

    m_drawCount += node->triangleCount;
}

void QuadTree::FindNode(NodeType *node, float positionX, float positionZ, float &height)
{
    float minX = node->positionX - node->width / 2.0f;
    float maxX = node->positionX + node->width / 2.0f;
    float minZ = node->positionZ - node->width / 2.0f;
    float maxZ = node->positionZ + node->width / 2.0f;

    if (positionX < minX || positionX > maxX || positionZ < minZ || positionZ > maxZ)
    {
        return;
    }

    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            count++;

            FindNode(node->nodes[i], positionX, positionZ, height);
        }
    }

    if (count != 0)
    {
        return;
    }

    std::thread t[8];

    auto size = node->meshData.indices.size();
    size -= (size % 3);

    for (int i = 0; i < 8; i++)
    {
        int start = i * size / 8;
        int end   = (i + 1) * size / 8;

        start -= (start % 3);

        t[i] = std::thread(&QuadTree::GetThreadTriangleHeight, this, node, start, end, positionX, positionZ, &height);
    }

    for (int i = 0; i < 8; i++)
    {
        t[i].join();
    }
}

void QuadTree::GetThreadTriangleHeight(NodeType *node, int start, int end, float positionX, float positionZ,
                                       float *height)
{
    for (int i = start; i < end; i += 3)
    {
        uint32_t i0 = node->meshData.indices[i];
        uint32_t i1 = node->meshData.indices[i + 1];
        uint32_t i2 = node->meshData.indices[i + 2];

        Vector3 v0 = node->meshData.vertices[i0].position;
        Vector3 v1 = node->meshData.vertices[i1].position;
        Vector3 v2 = node->meshData.vertices[i2].position;

        if (GetTriangleHeight(v0, v1, v2, positionX, positionZ, *height))
        {
            return;
        }
    }
}

bool QuadTree::GetTriangleHeight(Vector3 v0, Vector3 v1, Vector3 v2, float positionX, float positionZ, float &height)
{
    Vector3 n = (v1 - v0).Cross(v2 - v1);
    n.Normalize();

    Vector3 o = Vector3(positionX, height, positionZ);
    Vector3 d = Vector3(0.0f, -1.0f, 0.0f);

    if (abs(n.Dot(d)) < 1e-5)
    {
        return false;
    }

    float t0 = (v0.Dot(n) - o.Dot(n)) / n.Dot(d);
    float t1 = (v1.Dot(n) - o.Dot(n)) / n.Dot(d);
    float t2 = (v2.Dot(n) - o.Dot(n)) / n.Dot(d);

    float t = XMMin(t0, XMMin(t1, t2));

    Vector3 p = o + t * d; // hit closest position.

    Vector3 a0 = (p - v1).Cross(v0 - v1);
    Vector3 a1 = (p - v2).Cross(v1 - v2);
    Vector3 a2 = (p - v0).Cross(v2 - v0);

    float alpha0 = a0.Dot(n);
    float alpha1 = a1.Dot(n);
    float alpha2 = a2.Dot(n);

    // 삼각형 내부에 hit position 이 존재하지 않는다면 예외
    if (alpha0 < 0.0f || alpha1 < 0.0f || alpha2 < 0.0f)
    {
        return false;
    }

    height = p.y + 2.0f;

    return true;
}
