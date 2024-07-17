#pragma once

class CommandContext;

class ContextManager
{
  public:
    ContextManager(void)
    {
    }

    CommandContext *AllocateContext(D3D12_COMMAND_LIST_TYPE type);
};

class CommandContext
{
    friend class ContextManager;
    friend class GraphicsContext;

  private:
    CommandContext(D3D12_COMMAND_LIST_TYPE type);

  public:
    ~CommandContext();

    void Initialize();

    static CommandContext &Begin(const std::wstring &ID);

    GraphicsContext& GetGraphicsContext()
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
};
