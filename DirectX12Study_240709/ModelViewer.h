#pragma once

#include "AppBase.h"

using namespace DirectX;

class Model;

class ModelViewer : public AppBase
{
  public:
    ModelViewer();
    virtual ~ModelViewer();

    virtual bool Initialize() override;

  protected:
    virtual void Update() override;
    virtual void Render() override;

  private:
    Model *m_model = nullptr;
};
