#pragma once

#include "Mesh.h"

class Model;
class Frustum;

class QuadTree
{
    friend class DebugQuadTree;

  private:
    struct NodeType
    {
        float positionX, positionZ, width;
        int triangleCount;
        Model *model = nullptr;
        MeshData meshData;
        NodeType *nodes[4];
    };

  public:
    ~QuadTree();

    void Initialize(Model *terrain, std::vector<MeshData> meshes, ID3D12Device *device,
                    ID3D12GraphicsCommandList *commandList);
    void ReleaseNode(NodeType *node);
    void Update();
    void Render(Frustum *frustum, ID3D12GraphicsCommandList *commandList);

    NodeType *GetRootNode()
    {
        return m_rootNode;
    }

    void GetHeight(float positionX, float positionZ, float &height);

  private:
    void CaculateMeshDimesion(float &centerX, float &centerZ, float &wdith);
    void CreateTreeNode(NodeType *node, float positionX, float positionZ, float width, ID3D12Device *device,
                        ID3D12GraphicsCommandList *commandList);
    int CountTriangles(float positionX, float positionZ, float width);
    void IsTriangleContained(int idx, float positionX, float positionZ, float width, MeshData meshData, bool *b,
                             int start, int end, int *count, MeshData *newMeshData);
    void UpdateNode(NodeType *node);
    void RenderNode(Frustum *frustum, NodeType *node, ID3D12GraphicsCommandList *commandList);
    void FindNode(NodeType *node, float positionX, float positionZ, float &height);

    void GetThreadTriangleHeight(NodeType *node, int start, int end, float positionX, float positionZ, float *height);
    bool GetTriangleHeight(Vector3 v0, Vector3 v1, Vector3 v2, float positionX, float positionZ, float &height);

  private:
    std::vector<MeshData> m_meshDatas;
    NodeType *m_rootNode = nullptr;
    uint32_t m_drawCount = 0;
};
