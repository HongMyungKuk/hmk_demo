#pragma once

class Command;

class EventHandler
{
  public:
    enum OBJ_COMMAND_TYPE
    {
        FRONT,
        BACK,
        LEFT,
        RIGHT,

        END,
    };

    EventHandler()
    {
        m_vecCommand.resize(END);
    }

    void RegistObjectMoveCommand(OBJ_COMMAND_TYPE type, Command *cmd)
    {
        m_vecCommand[type] = cmd;
    }

    void EventHandler::ObjectMoveHandle(OBJ_COMMAND_TYPE type, const float dt);

  private:
    std::vector<Command *> m_vecCommand;
};
