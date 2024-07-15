#pragma once

#include "ConstantBuffer.h"
#include "Mesh.h"
#include "AnimationData.h"

class GeometryGenerator
{
  public:
    static MeshData MakeSquare(const float w, const float h);
    static MeshData MakeCube(const float w, const float h, const float d);
    static MeshData MakeSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

    static auto ReadFromModelFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, std::vector<MaterialConsts>>;
    static auto ReadFromAnimationFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, AnimationData>;
};
