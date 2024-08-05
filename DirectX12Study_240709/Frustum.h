#pragma once

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

class Frustum
{
  public:
    void ConstructFrustum(float screenDepth, Matrix viewMatrix, Matrix projectionMatrix);

    bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);

  private:
    Vector4 m_plane[6];
};
