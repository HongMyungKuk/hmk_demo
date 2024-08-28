#pragma once

#include "Model.h"

class OceanModel : public Model
{
  public:
    OceanModel(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<MeshData>& mesh)
    {
        Initialize(device, cmdList, mesh);
    }

    virtual ID3D12PipelineState *GetPSO(bool isWire)
    {
        return isWire ? Graphics::defaultWirePSO : Graphics::oceanPSO;
    }
};
