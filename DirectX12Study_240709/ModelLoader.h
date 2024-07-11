#pragma once

#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
    void LoadModel(const char *filename);
    void ProcessNode(aiNode *node, const aiScene *scene);
    MeshData ProceesMesh(aiMesh *mesh, const aiScene *scene);

  private:
    std::vector<MeshData> m_meshes;
};
