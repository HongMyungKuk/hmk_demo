#pragma once

#include "ConstantBuffer.h"
#include "Mesh.h"

class PostEffects
{
  public:
    ~PostEffects()
    {
        SAFE_RELEASE(mesh.vertexBuffer);
        SAFE_RELEASE(mesh.indexBuffer);
    }

    void Initialize();
    void Update(const GlobalConsts &data);
    void Render(ID3D12GraphicsCommandList *commandList);

  private:
    Mesh mesh             = {};
    uint32_t m_indexCount = 0;
};
