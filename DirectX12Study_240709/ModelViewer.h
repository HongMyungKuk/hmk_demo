#pragma once

#include "AppBase.h"

using namespace DirectX;

class Camera;
class Model;
class SkinnedMeshModel;

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
    void ChangeModel();

  private:
    Model *m_ground = nullptr;
    Model *m_box    = nullptr;
    SkinnedMeshModel *m_model  = nullptr;
    Model *m_coordController = nullptr;

    std::string m_openModelFileBasePath = "";
    std::string m_openModelFileName     = "";

    // Test
    Vector3 m_controllerCenter      = Vector3(2.5f, 1.5f, 5.0f);
    BoundingSphere m_boundingSphere = {};
};
