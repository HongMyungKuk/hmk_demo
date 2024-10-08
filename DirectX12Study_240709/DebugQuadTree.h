#pragma once

#include "Terrain.h"

class Model;

class DebugQuadTree
{
  public:
    ~DebugQuadTree();

    void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, Terrain *quadTree, std::vector<Model*>& opaqueLists);
    void Update();
    void Render(ID3D12GraphicsCommandList *commandList);

  private:
    void CreateCubeMeshs(Terrain::QuadTree *node, std::vector<Model*>& opaqueLists);

  private:
    //std::vector<Model *> m_modelList;
    ID3D12Device *m_device;
    ID3D12GraphicsCommandList *m_commandList;
};
