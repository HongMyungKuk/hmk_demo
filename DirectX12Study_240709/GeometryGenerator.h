#pragma once

#include "AnimationData.h"
#include "ConstantBuffer.h"
#include "Mesh.h"

class GeometryGenerator
{
  public:
    static MeshData MakeSquare(const float w, const float h);
    static MeshData GeometryGenerator::MakeSquareGrid(const int numSlices, const int numStacks, const float scale,
                                                      const Vector2 texScale);
    static MeshData MakeCube(const float w, const float h, const float d);
    static MeshData MakeSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

    static auto ReadFromModelFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, std::vector<MaterialConsts>>;
    static auto ReadFromAnimationFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, AnimationData>;
};
