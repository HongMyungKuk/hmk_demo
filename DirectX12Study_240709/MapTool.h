#pragma once

#include "AppBase.h"

class Model;
class QuadTree;
class Frustum;
class DebugQuadTree;

class MapTool : public AppBase
{
  public:
    MapTool();
    virtual ~MapTool();
    virtual bool Initialize() override;

  protected:
    virtual void Update(const float dt) override;
    virtual void UpdateLights() override;
    virtual void Render() override;
    virtual void UpdateGui(const float frameRate) override;

  private:
    std::string m_basePath = "";
    Model *m_terrain       = nullptr;
    Model *m_skybox        = nullptr;

    // light on/off flag
    bool m_useDL = true;  // directional light
    bool m_usePL = false; // point lihgt
    bool m_useSL = false; // spot light

    QuadTree *m_quadTree = nullptr;
    Frustum *m_frustum   = nullptr;
    DebugQuadTree *m_DebugQaudTree = nullptr;
};
