#pragma once

#include "Mesh.h"

class Model;

class QuadTree
{
  private:
    struct NodeType
    {
        float positionX, positionZ, width;
        int triangleCount;
        Mesh mesh;
        NodeType *nodes[4];
    };

  public:
    void Initialize(Model *terrain, std::vector<MeshData> meshes);

  private:
    void CaculateMeshDimesion(float &centerX, float &centerZ, float &wdith);

  private:
    std::vector<MeshData> m_meshes;
};
