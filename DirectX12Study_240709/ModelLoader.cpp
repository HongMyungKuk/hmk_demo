#include "pch.h"

#include "ModelLoader.h"
#include <filesystem>
#include <fstream>

ModelLoader::ModelLoader(const char *filepath, const char *filename)
{
    basePath = std::string(filepath);

    uint8_t *fileFullpath = Utils::get_full_directory(filepath, filename);
    const uint8_t *ext    = Utils::get_extension((const char *)fileFullpath);
    if (!strcmp((const char *)ext, "obj"))
    {
        LoadObjFile((const char *)fileFullpath);
    }
    else
    {
        LoadModel((const char *)fileFullpath);
    }

    free((void*)ext);
    free(fileFullpath);
}

ModelLoader::~ModelLoader()
{
}

void ModelLoader::LoadObjFile(const char *filename)
{
    FILE *fp = nullptr;

    fopen_s(&fp, filename, "rt");
    if (!fp)
    {
        return;
    }

    char line[255];

    // read info.
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    MeshData meshData                       = {};
    std::vector<Vertex> &vertices           = meshData.vertices;
    std::vector<MeshData::index_t> &indices = meshData.indices;
    while (!feof(fp))
    {
        char c;
        c = fgetc(fp);

        switch (c)
        {
        case 'v': // vertex data.
        {
            XMFLOAT3 pos    = {};
            XMFLOAT3 normal = {};
            XMFLOAT2 uv     = {};

            c = fgetc(fp);
            if (c == ' ') // position data.
            {
                fscanf_s(fp, "%f %f %f", &pos.x, &pos.y, &pos.z);
            }
            else if (c == 'n') // normal data.
            {
                fscanf_s(fp, "%f %f %f", &normal.x, &normal.y, &normal.z);
            }
            else if (c == 't') // texture data.
            {
                fscanf_s(fp, "%f %f", &uv.x, &uv.y);
            }
            Vertex v = Vertex(pos, normal, uv);
            vertices.push_back(v);
        }
        break;
        case 'f': // index data.
        {
            MeshData::index_t idx[12] = {};

            for (int32_t i = 0; i < 12; i += 3)
            {
                int ans = fscanf_s(fp, "%d/%d/%d", &idx[i], &idx[i + 1], &idx[i + 2]);
                // std::cout << ans << std::endl;
                if (ans == 3)
                {
                    // std::cout << idx[i] << "," << idx[i + 1] << "," << idx[i + 2] << " ";
                    indices.push_back(idx[i]);
                    indices.push_back(idx[i + 1]);
                    indices.push_back(idx[i + 2]);
                }
                // else if(ans == 0){
                //     std::cout << ans << std::endl;
                //     //std::cout << idx[i] << "," << idx[i + 1] << "," << idx[i + 2] << std::endl;
                // }
                // else {
                //     std::cout << ans << std::endl;
                //     std::cout << idx[i] << "," << idx[i + 1] << "," << idx[i + 2] << std::endl;
                // }
            }
            // std::cout << std::endl;
        }
        break;
        case 'o': {
            fgets(line, sizeof(line), fp); // TODO

            m_meshes.push_back(meshData);

            vertices.clear();
            indices.clear();
        }
        break;
        case 's': {
            fgets(line, sizeof(line), fp);
            fgets(line, sizeof(line), fp);
            // TODO
        }
        break;
        }
    }

    m_meshes.push_back(meshData);

    fclose(fp);
}

void ModelLoader::LoadModel(const char *filename)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(filename, aiProcess_Triangulate | aiProcess_MakeLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    ProcessNode(scene->mRootNode, scene);
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProceesMesh(mesh, scene);
        m_meshes.push_back(newMesh);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

MeshData ModelLoader::ProceesMesh(aiMesh *mesh, const aiScene *scene)
{
    MeshData meshData;

    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 texCoord;

    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;

        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        v.position = position;

        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        v.normal = normal;

        if (mesh->mTextureCoords[0])
        {
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            v.texCoord = texCoord;
        }
        else
        {
            v.texCoord = XMFLOAT2(0.0f, 0.0f);
        }

        meshData.vertices.push_back(v);
    }
    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            meshData.indices.push_back(face.mIndices[j]);
    }

    // material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        MaterialConsts mat = {};

        aiColor3D ambient(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        aiColor3D diffuse(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        aiColor3D specular(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

        // ¸ðµ¨¿¡ ambient, diffuse, specular ¸¦ ³Ñ°ÜÁÜ.
        mat.ambient.x = ambient.r;
        mat.ambient.y = ambient.b;
        mat.ambient.z = ambient.g;

        mat.diffuse.x = diffuse.r;
        mat.diffuse.y = diffuse.b;
        mat.diffuse.z = diffuse.g;

        mat.specular.x = specular.r;
        mat.specular.y = specular.b;
        mat.specular.z = specular.g;

        m_materials.push_back(mat);

        for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
        {
            aiString str;
            material->GetTexture(aiTextureType_DIFFUSE, i, &str);
            const aiTexture *texture = scene->GetEmbeddedTexture(str.C_Str());
            if (texture)
            {
                if (texture->CheckFormat("png") || texture->CheckFormat("jpg"))
                {
                    std::string filename = std::string(std::filesystem::path(texture->mFilename.C_Str()).filename().string());
                    meshData.albedoTextureFilename = basePath + filename;
                    // std::cout << basePath + filename << std::endl;
                    {
                        std::ofstream os;
                        os.open(basePath + filename, std::ios::binary | std::ios::out);
                        os.write((char*)texture->pcData ,texture->mWidth);
                        os.close();
                    }
                }
            }
        }
    }

    return meshData;
}
