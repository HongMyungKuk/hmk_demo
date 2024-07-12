#pragma once

#include "Mesh.h"
#include "ConstantBuffer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelLoader
{
  public:
    ModelLoader()
    {
    }
    ModelLoader(const char* filepath, const char *filename);
    ~ModelLoader();

  public:
    std::vector<MeshData> Meshes()
    {
        return m_meshes;
    }
    std::vector<MaterialConsts> Materials()
    {
        return m_materials;
    }

  private:
    void LoadObjFile(const char *filename);
    void LoadModel(const char *filename);
    void ProcessNode(aiNode *node, const aiScene *scene);
    MeshData ProceesMesh(aiMesh *mesh, const aiScene *scene);

  private:
    std::string basePath = "";
    std::vector<MeshData> m_meshes;
    std::vector<MaterialConsts> m_materials;
};
