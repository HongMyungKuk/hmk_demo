#pragma once

#include "Mesh.h"

class GeometryGenerator
{
  public:
    static MeshData MakeSquare(const float w, const float h);
    static MeshData MakeCube(const float w, const float h, const float d);
};
