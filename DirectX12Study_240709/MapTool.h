#pragma once

#include "AppBase.h"

class Model;

class MapTool : public AppBase
{
  public:
    MapTool();
    virtual ~MapTool();
    virtual bool Initialize() override;

  protected:
    virtual void Update(const float dt) override;
    virtual void Render() override;
    virtual void UpdateGui(const float frameRate) override;

  private:
    std::string m_basePath = "";
    Model *m_terrain       = nullptr;
    Model *m_skybox        = nullptr;
};
