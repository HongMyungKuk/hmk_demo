#pragma once

class Command;

class EventHandler
{
  public:
    enum COMMAND_TYPE
    {
        OBJ,
        END,
    };

    EventHandler()
    {
        m_vecCommand.resize(END);
    }

    void RegistObjectMoveCommand(COMMAND_TYPE type, Command *cmd)
    {
        m_vecCommand[type] = cmd;
    }

    void ObjectMoveHandle(COMMAND_TYPE type);
    void Excute(COMMAND_TYPE type, const float dt);

  private:
    std::vector<Command *> m_vecCommand;
};
