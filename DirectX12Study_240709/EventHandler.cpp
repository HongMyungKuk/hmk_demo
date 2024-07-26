#include "pch.h"

#include "EventHandler.h"
#include "Command.h"

void EventHandler::ObjectMoveHandle(OBJ_COMMAND_TYPE type, const float dt)
{
    m_vecCommand[type]->Excute(dt);
}