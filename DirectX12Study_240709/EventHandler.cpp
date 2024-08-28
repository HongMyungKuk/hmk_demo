#include "pch.h"

#include "EventHandler.h"
#include "Command.h"

void EventHandler::SetHandler(COMMAND_TYPE type)
{

}

void EventHandler::Excute(COMMAND_TYPE type, const float dt)
{
    m_vecCommand[type]->Excute(dt);
}
