#pragma once

#include "QuadTree.h"

class Model;

class DebugQuadTree
{
  public:
    void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, QuadTree *quadTree);
    void Update();
    void Render(ID3D12GraphicsCommandList *commandList);

  private:
    void CreateCubeMeshs(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, QuadTree::NodeType *node);

  private:
    std::vector<Model *> m_modelList;
};
