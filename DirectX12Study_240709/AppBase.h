#pragma once

class AppBase
{
  public:
    AppBase();
    virtual ~AppBase();

    int32_t Run();

  protected:
    virtual bool Initialize();
    virtual void Render();
    virtual void Update();

    void WaitForPreviousFrame();

  private:
    bool InitWindow();
    bool InitD3D();
    bool InitGui();
    void GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter,
                            bool requestHighPerformanceAdapter = false);

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  public:
    static float GetAspect()
    {
        return (float)s_screenWidth / s_screenHeight;
    }

  protected:
    static const uint32_t s_frameCount = 2;

    // Pipeline objects.
    D3D12_VIEWPORT m_viewport                     = {};
    D3D12_RECT m_scissorRect                      = {};
    IDXGISwapChain *m_swapChain                   = nullptr;
    ID3D12Device *m_device                        = nullptr;
    ID3D12CommandAllocator *m_commandAllocator    = nullptr;
    ID3D12CommandQueue *m_commandQueue            = nullptr;
    ID3D12DescriptorHeap *m_rtvHeap               = nullptr;
    ID3D12DescriptorHeap *m_dsvHeap               = nullptr;
    ID3D12GraphicsCommandList *m_commandList      = nullptr;
    ID3D12Resource *m_depthStencilBuffer          = nullptr;
    ID3D12Resource *m_renderTargets[s_frameCount] = {
        nullptr,
    };
    uint32_t m_rtvDescriptorSize;
    uint32_t m_dsvDescriptorSize;

    // Synchronization objects.
    uint32_t m_frameIndex = 0;
    HANDLE m_fenceEvent   = nullptr;
    ID3D12Fence *m_fence  = nullptr;
    uint64_t m_fenceValue = 0;

    HWND m_hwnd          = nullptr;
    bool m_useWarpDevice = false;

  public:
    static const uint32_t s_screenWidth  = 1920;
    static const uint32_t s_screenHeight = 1080;
};
