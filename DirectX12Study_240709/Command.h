#pragma once

#include "Camera.h"
#include "SkinnedMeshModel.h"

class Command
{
  public:
    virtual void Excute(const float dt) = 0;
    virtual void Set()                  = 0;
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

    virtual void Set(OBJ_TYPE type)
    {
        switch (type)
        {
        case FRONT:
            if (m_camera)
            {
                m_model->AddVelocity(m_camera->GetDirection());
            }
            break;
        case BACK :
            if (m_camera)
            {
                m_model->AddVelocity(-m_camera->GetDirection());
            }
            break;
        case RIGHT:
            if (m_camera)
            {
                m_model->AddVelocity(m_camera->GetRightDirection());
            }
            break;
        case LEFT:
            if (m_camera)
            {
                m_model->AddVelocity(-m_camera->GetRightDirection());
            }
            break;
        }
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

  protected:
    Model *m_model   = nullptr;
    Light *m_light   = nullptr;
    Camera *m_camera = nullptr;

    float m_speed = 0.0f;
};

// Object move command.
class ObjectMoveBackCommand : public ObjectCommand
{
  public:
    ObjectMoveBackCommand(Model *model, Light *light, Camera *camera = nullptr) : ObjectCommand(model, light, camera)
    {
    }
};

class ObjectMoveFrontCommand : public ObjectCommand
{
  public:
    ObjectMoveFrontCommand(Model *model, Light *light, Camera *camera = nullptr) : ObjectCommand(model, light, camera)
    {
    }
};

class ObjectMoveLeftCommand : public ObjectCommand
{
  public:
    ObjectMoveLeftCommand(Model *model, Light *light, Camera *camera = nullptr) : ObjectCommand(model, light, camera)
    {
    }
};

class ObjectMoveRightCommand : public ObjectCommand
{
  public:
    ObjectMoveRightCommand(Model *model, Light *light, Camera *camera = nullptr) : ObjectCommand(model, light, camera)
    {
    }
};

// Light move command
class LightMoveBackCommand : public Command
{
};

class LightMoveFrontCommand : public Command
{
};

class LightMoveLeftCommand : public Command
{
};

class LightMoveRightCommand : public Command
{
};