#pragma once

#include "SkinnedMeshModel.h"

class Command
{
  public:
    virtual void Excute(const float dt) = 0;
};

class ObjectCommand : public Command
{
  public:
    ObjectCommand(Model *model, Light *light) : m_model(model), m_light(light)
    {
    }

    virtual void Excute(const float dt) = 0;

  protected:
    Model *m_model = nullptr;
    Light *m_light = nullptr;
};

// Object move command.
class ObjectMoveBackCommand : public ObjectCommand
{
  public:
    ObjectMoveBackCommand(Model *model, Light *light) : ObjectCommand(model, light)
    {
    }

    virtual void Excute(const float dt)
    {
        m_model->MoveBack(dt);
        m_light->position =
            Vector3::Transform(m_light->position, Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 1.0f) * dt *
                                                                            m_model->GetSpeed(Model::MOVE_TYPE::BACK)));
    }
};

class ObjectMoveFrontCommand : public ObjectCommand
{
  public:
    ObjectMoveFrontCommand(Model *model, Light *light) : ObjectCommand(model, light)
    {
    }

    virtual void Excute(const float dt)
    {
        m_model->MoveFront(dt);
        m_light->position = Vector3::Transform(
            m_light->position,
            Matrix::CreateTranslation(Vector3(0.0f, 0.0f, -1.0f) * dt * m_model->GetSpeed(Model::MOVE_TYPE::FRONT)));
    }
};

class ObjectMoveLeftCommand : public ObjectCommand
{
  public:
    ObjectMoveLeftCommand(Model *model, Light *light) : ObjectCommand(model, light)
    {
    }

    virtual void Excute(const float dt)
    {
        m_model->MoveLeft(dt);
        m_light->position =
            Vector3::Transform(m_light->position, Matrix::CreateTranslation(Vector3(1.0f, 0.0f, 0.0f) * dt *
                                                                            m_model->GetSpeed(Model::MOVE_TYPE::SIDE)));
    }
};

class ObjectMoveRightCommand : public ObjectCommand
{
  public:
    ObjectMoveRightCommand(Model *model, Light *light) : ObjectCommand(model, light)
    {
    }

    virtual void Excute(const float dt)
    {
        m_model->MoveRight(dt);
        m_light->position =
            Vector3::Transform(m_light->position, Matrix::CreateTranslation(Vector3(-1.0f, 0.0f, 0.0f) * dt *
                                                                            m_model->GetSpeed(Model::MOVE_TYPE::SIDE)));
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