#pragma once

#include "Mesh.h"

class ModelLoader
{
  public:
    ModelLoader()
    {
    }
    ModelLoader(const char *filename);
    ~ModelLoader();

  public:
    std::vector<MeshData> Meshes()
    {
        return m_meshes;
    }

  private:
    void LoadObjFile(const char *filename);

  private:
    std::vector<MeshData> m_meshes;
};
