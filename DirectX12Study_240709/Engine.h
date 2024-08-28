#pragma once

#include "AppBase.h"

class Model;
class Terrain;
// class QuadTree;
class Frustum;
class DebugQuadTree;

class Engine : public AppBase
{
  public:
    Engine();
    virtual ~Engine();
    virtual bool Initialize() override;

  protected:
    virtual void Update(const float dt) override;
    virtual void UpdateLights() override;
    virtual void Render() override;
    virtual void UpdateGui(const float frameRate) override;

  private:
    std::string m_basePath = "";
    // Model *m_terrain       = nullptr;
    Model *m_skybox = nullptr;

    // light on/off flag
    bool m_useDL = true;  // directional light
    bool m_usePL = false; // point lihgt
    bool m_useSL = false; // spot light

    Terrain *m_terrain    = nullptr;
    Model *m_temple       = nullptr;
    Model *m_billBoardSun = nullptr;
    Model *m_ocean        = nullptr;
    // QuadTree *m_quadTree = nullptr;
    Frustum *m_frustum             = nullptr;
    DebugQuadTree *m_DebugQaudTree = nullptr;

    bool m_isDebugTreeFlag = false;

    ID3D12Resource *m_uploadResource     = nullptr;
    ID3D12Resource *m_terrainTexResource = nullptr;

    float m_height = 0.0f;
};
