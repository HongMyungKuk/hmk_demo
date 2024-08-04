#include "pch.h"

#include "Model.h"
#include "QuadTree.h"

void QuadTree::Initialize(Model *terrain, std::vector<MeshData> meshes)
{
    float centerX = 0.0f;
    float centerZ = 0.0f;
    float width   = 0.0f;

    m_meshes = meshes;

    CaculateMeshDimesion(centerX, centerZ, width);
}

void QuadTree::CaculateMeshDimesion(float &centerX, float &centerZ, float &width)
{
    centerX = 0.0f;
    centerZ = 0.0f;

    Vector3 minPos(1000.0f, 1000.0f, 1000.0f);
    Vector3 maxPos(-1000.0f, -1000.0f, -1000.0f);
    for (auto m : m_meshes)
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
    centerZ = (minPos.y + maxPos.y) * 0.5f;

    float widthX = maxPos.x - minPos.x;
    float widthZ = maxPos.y - minPos.y;

    width = DirectX::XMMax(widthX, widthZ);
}
