#pragma once

#include "AppBase.h"

using namespace DirectX;

class Camera;
class Model;

class ModelViewer : public AppBase
{
  public:
    ModelViewer();
    virtual ~ModelViewer();
    virtual bool Initialize() override;

  protected:
    virtual void Update(const float dt) override;
    virtual void Render() override;
    virtual void UpdateGui(const float frameRate) override;

  private:
    Model *m_ground  = nullptr;
    Model *m_box     = nullptr;
    Model *m_model   = nullptr;
};
