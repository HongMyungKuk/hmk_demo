#include "pch.h"

#include "Model.h"
#include "Terrain.h"
#include "Frustum.h"

#define TRIANGLE_MAX_COUNT 5000

void Terrain::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *cmdList, std::vector<MeshData> meshData, std::vector<Model*>& opaqueLists)
{
    m_device      = device;
    m_commandList = cmdList;

    const MeshData &m = meshData[0];

    // 버텍스의 최소값과 최댓값 찾기
    Vector2 minV = Vector2(FLT_MAX, FLT_MAX);
    Vector2 maxV = Vector2(FLT_MIN, FLT_MIN);

    for (const auto &m : meshData)
    {
        for (const auto &v : m.vertices)
        {
            minV.x = XMMin(minV.x, v.position.x);
            minV.y = XMMin(minV.y, v.position.z);
            maxV.x = XMMax(maxV.x, v.position.x);
            maxV.y = XMMax(maxV.y, v.position.z);
        }
    }

    float lenX = maxV.x - minV.x;
    float lenY = maxV.y - minV.y;

    float cx     = (minV.x + maxV.x) * 0.5f;
    float cz     = (minV.y + maxV.y) * 0.5f;
    float radius = XMMax(lenX, lenY) * 0.5f;

    InitDivideQuad(&m_rootNode, cx, cz, radius, m, opaqueLists);
}

void Terrain::Destroy()
{
    DestroyNode(m_rootNode);
}

void Terrain::Render(Frustum *frustum)
{
    m_frustum = frustum;

    m_meshCompRenderCount = 0;

    RenderNode(m_rootNode);

    // std::cout << m_meshCompRenderCount << std::endl;
}

void Terrain::Update()
{
    UpdateNode(m_rootNode);
}

void Terrain::InitDivideQuad(QuadTree **node, const float cx, const float cz, const float radius, const MeshData &m, std::vector<Model*>& opaqueLists)
{
    *node = new QuadTree;
    assert(node);

    (*node)->cx     = cx;
    (*node)->cz     = cz;
    (*node)->radius = radius;

    MeshData _m = GetTriangeInArea(cx, cz, radius, m);

    int32_t triangleCount = GetTriangleCount(cx, cz, radius, m);
    std::cout << triangleCount << std::endl;

    int32_t parentPassFlag = 0;
    // 삼각형의 갯수가 초과하면 Tree에 공간을 분할하여 저장한다.
    if (triangleCount > TRIANGLE_MAX_COUNT)
    {
        InitDivideQuad(&(*node)->child[0], cx - radius * 0.5f, cz - radius * 0.5f, radius * 0.5f, _m, opaqueLists);
        InitDivideQuad(&(*node)->child[1], cx - radius * 0.5f, cz + radius * 0.5f, radius * 0.5f, _m, opaqueLists);
        InitDivideQuad(&(*node)->child[2], cx + radius * 0.5f, cz - radius * 0.5f, radius * 0.5f, _m, opaqueLists);
        InitDivideQuad(&(*node)->child[3], cx + radius * 0.5f, cz + radius * 0.5f, radius * 0.5f, _m, opaqueLists);

        parentPassFlag++;
    }

    // 자식 노드에서 이미 처리가 된 경우
    if (parentPassFlag != 0)
    {
        return;
    }

    (*node)->meshData = _m;
    // 각 노드의 컴포넌트 추가
    (*node)->model = new Model;
    (*node)->model->Initialize(m_device, m_commandList, {_m}, {}, true);
    (*node)->model->GetMaterialConstCPU().useAlbedoMap = true;
    (*node)->model->GetMaterialConstCPU().metalnessFactor = 0.0f;
    (*node)->model->GetMaterialConstCPU().roughnessFactor = 1.0f;
    (*node)->model->m_isDraw = false;

    opaqueLists.push_back((*node)->model);

    m_meshCompCount++;
}

MeshData Terrain::GetTriangeInArea(float cx, float cz, float radius, const MeshData &m)
{
    MeshData meshData;

    const float minX = cx - radius, maxX = cx + radius;
    const float minZ = cz - radius, maxZ = cz + radius;
    uint32_t idx = 0;
    for (size_t i = 0; i < m.indices.size(); i += 3)
    {
        auto i0 = m.indices[i];
        auto i1 = m.indices[i + 1];
        auto i2 = m.indices[i + 2];

        auto v0 = m.vertices[i0];
        auto v1 = m.vertices[i1];
        auto v2 = m.vertices[i2];

        auto p0 = v0.position;
        auto p1 = v1.position;
        auto p2 = v2.position;

        // 포함되지 않는 삼각형
        if (((p0.x > maxX || p0.x < minX) || (p0.z > maxZ || p0.z < minZ)) &&
            ((p1.x > maxX || p1.x < minX) || (p1.z > maxZ || p1.z < minZ)) &&
            ((p2.x > maxX || p2.x < minX) || (p2.z > maxZ || p2.z < minZ)))
        {
            continue;
        }

        meshData.vertices.push_back(v0);
        meshData.vertices.push_back(v1);
        meshData.vertices.push_back(v2);

        meshData.indices.push_back(idx);
        meshData.indices.push_back(idx + 1);
        meshData.indices.push_back(idx + 2);

        idx += 3;
    }

    return meshData;
}

int32_t Terrain::GetTriangleCount(float cx, float cz, float radius, const MeshData &m)
{
    int32_t count = 0;

    const float minX = cx - radius, maxX = cx + radius;
    const float minZ = cz - radius, maxZ = cz + radius;
    for (size_t i = 0; i < m.indices.size(); i += 3)
    {
        auto i0 = m.indices[i];
        auto i1 = m.indices[i + 1];
        auto i2 = m.indices[i + 2];

        auto v0 = m.vertices[i0];
        auto v1 = m.vertices[i1];
        auto v2 = m.vertices[i2];

        auto p0 = v0.position;
        auto p1 = v1.position;
        auto p2 = v2.position;

        // 포함되지 않는 삼각형
        if (((p0.x > maxX || p0.x < minX) || (p0.z > maxZ || p0.z < minZ)) &&
            ((p1.x > maxX || p1.x < minX) || (p1.z > maxZ || p1.z < minZ)) &&
            ((p2.x > maxX || p2.x < minX) || (p2.z > maxZ || p2.z < minZ)))
        {
            continue;
        }

        count++;
    }

    return count;
}

void Terrain::GetHeight(QuadTree *node, float x, float z, float *height)
{
    float minX = node->cx - node->radius, maxX = node->cx + node->radius;
    float minZ = node->cz - node->radius, maxZ = node->cz + node->radius;

    if ((x > maxX || x < minX) || (z > maxZ || z < minZ))
    {
        return;
    }

    int32_t paraentPassFlag = 0;

    for (int32_t i = 0; i < 4; i++)
    {
        if (node->child[i] == nullptr)
            continue;

        paraentPassFlag++;

        GetHeight(node->child[i], x, z, height);
    }

    if (paraentPassFlag != 0)
    {
        return;
    }

    for (size_t i = 0; i < node->meshData.vertices.size(); i += 3)
    {
        auto v0 = node->meshData.vertices[i].position;
        auto v1 = node->meshData.vertices[i + 1].position;
        auto v2 = node->meshData.vertices[i + 2].position;

        auto normal = (v1 - v0).Cross(v2 - v0);
        normal.Normalize();

        if (IsinsideTriangle(v0, v1, v2, normal, x, z, height) == true)
        {
            return;
        }
    }
}

bool Terrain::IsinsideTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 n, float x, float z, float *height)
{
    // y 좌표 임시로 설정.
    Vector3 o = Vector3(x, 10.0f, z);
    Vector3 d = Vector3(0.0f, -1.0f, 0.0f);

    float t0 = (v0.Dot(n) - o.Dot(n)) / d.Dot(n);
    float t1 = (v1.Dot(n) - o.Dot(n)) / d.Dot(n);
    float t2 = (v2.Dot(n) - o.Dot(n)) / d.Dot(n);

    // std::cout << t0 << " " << t1 << " " << t2 << std::endl;

    Vector3 p = o + t0 * d;

    auto det0 = (p - v1).Cross(v0 - v1);

    auto det1 = (p - v2).Cross(v1 - v2);

    auto det2 = (p - v0).Cross(v2 - v0);

    float alpha = 0.0f;
    alpha       = det0.Dot(det1);
    if (alpha < 0)
    {
        return false;
    }

    alpha = det1.Dot(det2);
    if (alpha < 0)
    {
        return false;
    }

    alpha = det0.Dot(det2);
    if (alpha < 0)
    {
        return false;
    }

    *height = p.y;

    return true;
}

void Terrain::UpdateNode(QuadTree *node)
{
    int32_t parentPaasFlag = 0;

    for (int32_t i = 0; i < 4; i++)
    {
        if (node->child[i] == nullptr)
            continue;

        parentPaasFlag++;

        UpdateNode(node->child[i]);
    }

    if (parentPaasFlag != 0)
    {
        return;
    }

    node->model->Update();
}

void Terrain::RenderNode(QuadTree *node)
{
    int32_t parentPaasFlag = 0;

    for (int32_t i = 0; i < 4; i++)
    {
        if (node->child[i] == nullptr)
            continue;

        parentPaasFlag++;

        RenderNode(node->child[i]);
    }

    if (parentPaasFlag != 0)
    {
        return;
    }

    if (m_frustum->CheckCube(node->cx, 0.0f, node->cz, node->radius) == true)
    {
        // node->model->Render(m_commandList);
        
        node->model->m_isDraw = true;
        m_meshCompRenderCount++;
    }
}

void Terrain::DestroyNode(QuadTree *node)
{
    int32_t parentPaasFlag = 0;

    for (int32_t i = 0; i < 4; i++)
    {
        if (node->child[i] == nullptr)
            continue;

        parentPaasFlag++;

        DestroyNode(node->child[i]);
    }

    if (parentPaasFlag != 0)
    {
        delete node;
        return;
    }

    // delete node->model; // opqaueList 에서 제거
    delete node;
}

void Terrain::GetObjectHeight(float x, float z, float *height)
{
    GetHeight(m_rootNode, x, z, height);
}
