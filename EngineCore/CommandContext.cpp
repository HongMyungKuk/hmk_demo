#include "pch.h"

#include "CommandContext.h"
#include "CommandListManager.h"
#include "GraphicsCore.h"

using namespace Graphics;

CommandContext *ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
{
    CommandContext *ret = nullptr;

    ret = new CommandContext(type);
    ret->Initialize();

    assert(ret);
    assert(ret->m_type == type);

    return ret;
}

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type) : m_type(type)
{
    m_cmdList         = nullptr;
    m_currentAlloctor = nullptr;
}

CommandContext::~CommandContext()
{
}

void CommandContext::Initialize()
{
    g_CommandManager.CreateNewCommandList(m_type, &m_cmdList, &m_currentAlloctor);
}

CommandContext &CommandContext::Begin(const std::wstring &ID = L"")
{
    CommandContext *newContext = g_ContextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
    newContext->SetID(ID);

    // 엔진 프로파일링 추가.

    return *newContext;
}
