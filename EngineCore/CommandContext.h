#pragma once

class CommandContext;
class ColorBuffer;

class ContextManager
{
  public:
    ContextManager(void)
    {
    }
    ~ContextManager()
    {
    }

    CommandContext *AllocateContext(D3D12_COMMAND_LIST_TYPE type);
    void FreeContext(CommandContext *usedContext);
    void DestroyAllContext();

  private:
    std::vector<CommandContext *> sm_contextPool[4];
    std::queue<CommandContext *> sm_availableContext[4]; // D3D12_COMMAND_LIST_TYPE
};

class CommandContext
{
    friend class ContextManager;
    friend class GraphicsContext;

  private:
    CommandContext(D3D12_COMMAND_LIST_TYPE type);

    void Reset();

  public:
    ~CommandContext();

    void Initialize();

    static void DestroyAllContext();

    static CommandContext &Begin(const std::wstring &ID);
    void Finish();

    void TransitionResource(ColorBuffer &res, D3D12_RESOURCE_STATES newState);

    GraphicsContext &GetGraphicsContext()
    {
        assert(m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE);
        return reinterpret_cast<GraphicsContext &>(*this);
    }

    inline void SetID(const std::wstring &ID)
    {
        m_ID = ID;
    }

  protected:
    std::wstring m_ID;
    D3D12_COMMAND_LIST_TYPE m_type;
    ID3D12GraphicsCommandList *m_cmdList;
    ID3D12CommandAllocator *m_currentAlloctor;
};

class GraphicsContext : public CommandContext
{
  public:
    static GraphicsContext &Begin(const std::wstring &ID = L"")
    {
        return CommandContext::Begin(ID).GetGraphicsContext();
    }

    void ClearColor(ColorBuffer &res);
    void SetRenderTargets(uint64_t numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
};
