#pragma once

#include "CommandContext.h"

class Model
{
  public:
    Model();
    ~Model();

    void Initialize();
    void Update(GraphicsContext &context);
    void Render(GraphicsContext &context);

  private:
    ID3D12Resource *m_deaultBuffer              = nullptr;
    ID3D12Resource *m_uploadBuffer              = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    ID3D12Resource *m_deaultBuffer2           = nullptr;
    ID3D12Resource *m_uploadBuffer2           = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
};
