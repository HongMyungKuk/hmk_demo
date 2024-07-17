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

    free((void *)ext);
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
    std::vector<XMFLOAT3> posVec            = {};
    std::vector<XMFLOAT3> normalVec         = {};
    std::vector<XMFLOAT2> uvVec             = {};
    while (!feof(fp))
    {
        char c;
        c = fgetc(fp);

        switch (c)
        {
        case 'v': // vertex data.
        {
            c = fgetc(fp);

            Vertex v;
            XMFLOAT3 pos    = {};
            XMFLOAT3 normal = {};
            XMFLOAT2 uv     = {};
            if (c == ' ') // position data.
            {
                fscanf_s(fp, "%f %f %f", &pos.x, &pos.y, &pos.z);
                v = Vertex(pos, normal, uv);
                vertices.push_back(v);
            }
        }
        break;
        case 'f': // index data.
        {
            MeshData::index_t idx[12] = {};
            for (int32_t i = 0; i < 12; i += 3)
            {
                int ans = fscanf_s(fp, "%d/%d/%d", &idx[i], &idx[i + 1], &idx[i + 2]);
                if (ans == 3)
                {
                    indices.push_back(idx[i]);
                    indices.push_back(idx[i + 1]);
                    indices.push_back(idx[i + 2]);
                }
            }
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
    const aiScene *scene = import.ReadFile(filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    // 매쉬에 영향을 주는 bone들의 목록을 초기화한다.
    FindDeformAnim(scene);

    // node의 순서대로 ID를 정렬한다.
    int count = 0;
    UpdateBoneIDs(scene->mRootNode, &count);

    m_anim.boneParentId.resize(m_anim.boneNameToId.size(), -1);

    ProcessNode(scene->mRootNode, scene);

    if (scene->HasAnimations())
        ReadAnimationClip(scene);
}

aiNode *ModelLoader::FindParent(aiNode *node)
{
    if (!node)
    {
        return nullptr;
    }
    if (m_anim.boneNameToId.count(node->mName.C_Str()) > 0)
    {
        return node;
    }
    return FindParent(node->mParent);
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene)
{
    if (node->mParent && m_anim.boneNameToId.count(node->mName.C_Str()) && FindParent(node->mParent))
    {
        auto boneID                 = m_anim.boneNameToId[node->mName.C_Str()];
        m_anim.boneParentId[boneID] = m_anim.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
    }

    Matrix m(&node->mTransformation.a1);
    m = m.Transpose();

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProceesMesh(mesh, scene);
        for (auto& v : newMesh.vertices)
        {
            v.position = Vector3::Transform(v.position, m);
        }
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

    if (mesh->HasBones())
    {
        std::vector<std::vector<float>> boneWeights(meshData.vertices.size());
        std::vector<std::vector<uint8_t>> boneIndices(meshData.vertices.size());
        for (unsigned int i = 0; i < mesh->mNumBones; i++)
        {
            const aiBone *bone = mesh->mBones[i];

            m_anim.offsetMatrix.resize(m_anim.boneNameToId.size());
            m_anim.boneTransform.resize(m_anim.boneNameToId.size());

            auto boneID = m_anim.boneNameToId[bone->mName.C_Str()];

            m_anim.offsetMatrix[boneID] = Matrix((float *)&bone->mOffsetMatrix).Transpose();

            for (unsigned int j = 0; j < bone->mNumWeights; j++)
            {
                aiVertexWeight weight = bone->mWeights[j];
                assert(weight.mVertexId < boneIndices.size());
                boneWeights[weight.mVertexId].push_back(weight.mWeight);
                boneIndices[weight.mVertexId].push_back(boneID);
            }
        }

        int maxBones = 0;
        for (size_t i = 0; i < boneWeights.size(); i++)
        {
            maxBones = DirectX::XMMax(maxBones, int(boneWeights[i].size()));
        }
        std::cout << "Max number of influencing bones per vertex = " << maxBones << std::endl;

        meshData.skinnedVertices.resize(meshData.vertices.size());
        for (size_t i = 0; i < meshData.vertices.size(); i++)
        {
            meshData.skinnedVertices[i].position = meshData.vertices[i].position;
            meshData.skinnedVertices[i].normal   = meshData.vertices[i].normal;
            meshData.skinnedVertices[i].texCoord = meshData.vertices[i].texCoord;

            for (size_t j = 0; j < boneWeights[i].size(); j++)
            {
                meshData.skinnedVertices[i].boneWeights[j] = boneWeights[i][j];
                meshData.skinnedVertices[i].boneIndices[j] = boneIndices[i][j];
            }
        }
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

        // 모델에 ambient, diffuse, specular 를 넘겨줌.
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
                    std::string filename =
                        std::string(std::filesystem::path(texture->mFilename.C_Str()).filename().string());
                    meshData.albedoTextureFilename = basePath + filename;
                    {
                        std::ofstream os;
                        os.open(meshData.albedoTextureFilename, std::ios::binary | std::ios::out);
                        os.write((char *)texture->pcData, texture->mWidth);
                        os.close();
                    }
                }
            }
        }
    }

    return meshData;
}

void ModelLoader::FindDeformAnim(const aiScene *scene)
{
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        if (mesh->HasBones())
        {
            for (unsigned int j = 0; j < mesh->mNumBones; j++)
            {
                aiBone *bone                             = mesh->mBones[j];
                m_anim.boneNameToId[bone->mName.C_Str()] = -1;
            }
        }
    }
}

void ModelLoader::UpdateBoneIDs(aiNode *node, int *count)
{
    if (node)
    {
        if (m_anim.boneNameToId.count(node->mName.C_Str()))
        {
            m_anim.boneNameToId[node->mName.C_Str()] = *count;
            (*count)++;
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            UpdateBoneIDs(node->mChildren[i], count);
        }
    }
}

void ModelLoader::ReadAnimationClip(const aiScene *scene)
{
    m_anim.clips.resize(scene->mNumAnimations);
    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation *ani = scene->mAnimations[i];

        auto &clip = m_anim.clips[i];

        clip.name          = ani->mName.C_Str();
        clip.duration      = ani->mDuration;
        clip.tickPerSecond = ani->mTicksPerSecond;
        clip.numChannels   = ani->mNumChannels;
        clip.keys.resize(m_anim.boneNameToId.size());

        for (unsigned int i = 0; i < ani->mNumChannels; i++)
        {
            const aiNodeAnim *nodeAnim = ani->mChannels[i];
            const auto boneID          = m_anim.boneNameToId[nodeAnim->mNodeName.C_Str()];
            clip.keys[boneID].resize(nodeAnim->mNumPositionKeys);
            for (unsigned int j = 0; j < nodeAnim->mNumPositionKeys; j++)
            {
                auto pos   = nodeAnim->mPositionKeys[j].mValue;
                auto rot   = nodeAnim->mRotationKeys[j].mValue;
                auto scale = nodeAnim->mScalingKeys[j].mValue;
                auto &key  = clip.keys[boneID][j];
                key.pos    = {pos.x, pos.y, pos.z};
                key.rot    = Quaternion(rot.x, rot.y, rot.z, rot.w);
                key.scale  = {scale.x, scale.y, scale.z};
            }
        }
    }
}
