#include "pch.h"

#include "ModelLoader.h"

ModelLoader::ModelLoader(const char *filename)
{
    const uint8_t *ext = Uitls::get_extension(filename);
    if (!strcmp((const char *)ext, "obj"))
    {
        LoadObjFile(filename);
    }
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
                fscanf_s(fp, "%d/%d/%d", &idx[i], &idx[i + 1], &idx[i + 2]);
                std::cout << idx[i] << "," << idx[i + 1] << "," << idx[i + 2] << " ";
                indices.push_back(idx[i + 2]);
                indices.push_back(idx[i + 1]);
                indices.push_back(idx[i]);
            }
            std::cout << std::endl;
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
