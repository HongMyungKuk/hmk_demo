#pragma once

#include "Camera.h"
#include "SkinnedMeshModel.h"

class Command
{
  public:
    virtual void Excute(const float dt) = 0;
};

class ObjectCommand : public Command
{
  public:
    enum OBJ_TYPE
    {
        FRONT,
        BACK,
        RIGHT,
        LEFT,
        END,
    };

    ObjectCommand(Model *model, Light *light, Camera *camera = nullptr)
        : m_model(model), m_light(light), m_camera(camera)
    {
    }

    virtual void Excute(const float dt)
    {
    }

  protected:
    Model *m_model   = nullptr;
    Light *m_light   = nullptr;
    Camera *m_camera = nullptr;

    float m_speed = 0.0f;
};

// Object move command.
class ObjectMoveCommand : public ObjectCommand
{
  public:
    ObjectMoveCommand(Model *model, Light *light, Camera *camera = nullptr) : ObjectCommand(model, light, camera)
    {
    }

    virtual void Excute(const float dt)
    {
        if (m_camera)
        {
            m_model->SetVelocity(-m_camera->GetDirection());
        }
        m_model->Move(dt);

        m_light[1].position = Vector3::Transform(
            m_light[1].position, Matrix::CreateTranslation(-m_camera->GetDirection() * dt * m_model->GetSpeed()));

        if (m_camera)
        {
            m_camera->SetCameraSpeed(m_model->GetSpeed());
            m_camera->SetDirection(m_model->GetVelocity());
            m_camera->MoveBack(dt);
        }
    }
};

class ObjectDirectionSetCommand : public ObjectCommand
{
  public:
    ObjectDirectionSetCommand(Model *model, Light *light, Camera *camera = nullptr)
        : ObjectCommand(model, light, camera)
    {
    }

    virtual void Excute(const float dt)
    {
        if (m_camera)
        {
            m_model->AddVelocity(m_camera->GetDirection());
        }
    }
};

// Light move command
class LightMoveCommand : public Command
{
};
