#pragma once

#include "Mesh.h"
#include "ConstantBuffer.h"

class GeometryGenerator
{
  public:
    static MeshData MakeSquare(const float w, const float h);
    static MeshData MakeCube(const float w, const float h, const float d);
    static auto ReadFromModelFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, std::vector<MaterialConsts>>;
};
