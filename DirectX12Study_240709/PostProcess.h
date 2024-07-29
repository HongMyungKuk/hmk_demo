#pragma once

#include "ConstantBuffer.h"
#include "Mesh.h"

class PostProcess
{
    struct Consts
    {
        float exposure;
        float gammeaFactor;
    };


  public:
    ~PostProcess()
    {
        SAFE_RELEASE(mesh.vertexBuffer);
        SAFE_RELEASE(mesh.indexBuffer);
    }

    void Initialize();
    void Update();
    void Render(ID3D12GraphicsCommandList *commandList);

    Consts &GetConstCPU()
    {
        return m_constsData;
    }

  private:
    Mesh mesh             = {};
    uint32_t m_indexCount = 0;

    Consts m_constsData;
    UploadBuffer<Consts> m_constsBuffer;
};
