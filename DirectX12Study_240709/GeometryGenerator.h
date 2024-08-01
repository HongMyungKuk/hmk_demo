#pragma once

#include "AnimationData.h"
#include "ConstantBuffer.h"
#include "Mesh.h"

class GeometryGenerator
{
  public:
    static MeshData MakeTriangle(const float x);
    static MeshData MakeTriangle(const Vector3 v0, const Vector3 v1, const Vector3 v2);
    static MeshData MakeSquare(const float w, const float h);
    static MeshData MakeSquareGrid(const int numSlices, const int numStacks, const float scale,
                                                      const Vector2 texScale);
    static MeshData GeometryGenerator::MakeCylinder(const float bottomRadius, const float topRadius, float height,
                                                    int numSlices);
    static MeshData MakeCube(const float w, const float h, const float d);
    static MeshData MakeSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

    static auto ReadFromModelFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, std::vector<MaterialConsts>>;
    static auto ReadFromAnimationFile(const char *filepath, const char *filename)
        -> std::pair<std::vector<MeshData>, AnimationData>;
};
