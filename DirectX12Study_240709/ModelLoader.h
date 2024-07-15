#pragma once

#include "AnimationData.h"
#include "ConstantBuffer.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class ModelLoader
{
  public:
    ModelLoader()
    {
    }
    ModelLoader(const char *filepath, const char *filename);
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
    AnimationData &Animation()
    {
        return m_anim;
    }

  private:
    void LoadObjFile(const char *filename);
    void LoadModel(const char *filename);
    void ProcessNode(aiNode *node, const aiScene *scene);
    MeshData ProceesMesh(aiMesh *mesh, const aiScene *scene);
    void FindDeformAnim(const aiScene *scene);
    void UpdateBoneIDs(aiNode *node, int* count);
    void ReadAnimationClip(const aiScene *scene);
    aiNode *FindParent(aiNode *node);

  private:
    std::string basePath = "";
    std::vector<MeshData> m_meshes;
    std::vector<MaterialConsts> m_materials;
    AnimationData m_anim = {};
};
