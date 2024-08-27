#include "pch.h"

#include "EventHandler.h"
#include "Command.h"

void EventHandler::ObjectMoveHandle(COMMAND_TYPE type, ObjectCommand::OBJ_TYPE objType)
{
    m_vecCommand[type]->Set(objType);
}

void EventHandler::Excute(COMMAND_TYPE type, const float dt)
{
    m_vecCommand[type]->Excute(dt);
}
