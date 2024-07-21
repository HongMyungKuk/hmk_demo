#include "pch.h"

#include "ColorBuffer.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "GraphicsCore.h"

using namespace Graphics;

CommandContext *ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
{
    CommandContext *ret = nullptr;

    auto &availableContext = sm_availableContext[type];

    if (availableContext.empty())
    {
        ret = new CommandContext(type);
        sm_contextPool->push_back(ret);
        ret->Initialize();
    }
    else
    {
        ret = availableContext.front();
        availableContext.pop();
        ret->Reset();
    }

    assert(ret);
    assert(ret->m_type == type);

    return ret;
}

void ContextManager::FreeContext(CommandContext *usedContext)
{
    assert(usedContext != nullptr);
    sm_availableContext[usedContext->m_type].push(usedContext);
}

void ContextManager::DestroyAllContext()
{
    for (uint32_t i = 0; i < 4; i++)
    {
        for (uint32_t j = 0; j < uint32_t(sm_contextPool[i].size()); j++) {
            SAFE_DELETE(sm_contextPool[i][j]);
        }
        sm_contextPool[i].clear();
    }
}

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type) : m_type(type)
{
    m_cmdList         = nullptr;
    m_currentAlloctor = nullptr;
}

CommandContext::~CommandContext()
{
    SAFE_RELEASE(m_cmdList);
    // alloctor도 여기서 지우면 나중에 Crash
}

void CommandContext::Initialize()
{
    g_CommandManager.CreateNewCommandList(m_type, &m_cmdList, &m_currentAlloctor);
}

void CommandContext::Reset()
{
    m_cmdList->Reset(m_currentAlloctor, nullptr);
}

void CommandContext::DestroyAllContext()
{
    g_ContextManager.DestroyAllContext();
}

void CommandContext::Finish()
{
    CommandQueue &cmdQueue = g_CommandManager.GetQueue();

    uint64_t fenceValue = cmdQueue.ExcuteCommandList(m_cmdList);

    g_CommandManager.WaitForFence(fenceValue);

    g_ContextManager.FreeContext(this);
}

void CommandContext::TransitionResource(ColorBuffer &res, D3D12_RESOURCE_STATES newState)
{
    D3D12_RESOURCE_STATES oldStates = res.m_usageState;

    m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res.GetResource(), oldStates, newState));
    res.m_usageState = newState;
}

CommandContext &CommandContext::Begin(const std::wstring &ID)
{
    CommandContext *newContext = g_ContextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
    newContext->SetID(ID);

    // 엔진 프로파일링 추가.

    return *newContext;
}

void GraphicsContext::ClearColor(ColorBuffer &res)
{
    const float clearColor[] = {0.5f, 0.5f, 0.5f, 1.0f};
    m_cmdList->ClearRenderTargetView(res.GetRTV(), clearColor, 0, nullptr);
}

void GraphicsContext::SetRenderTargets(uint64_t numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[])
{
    m_cmdList->OMSetRenderTargets(UINT(numRTVs), RTVs, false, nullptr);
}