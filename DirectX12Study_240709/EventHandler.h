#pragma once

class Command;

class EventHandler
{
  public:
    enum COMMAND_TYPE
    {
        OBJ_DIRECTION_SET,
        OBJ_MOVE,
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

    void SetHandler(COMMAND_TYPE type);
    void Excute(COMMAND_TYPE type, const float dt);

  private:
    std::vector<Command *> m_vecCommand;
};
