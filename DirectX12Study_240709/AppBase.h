#pragma once

#include "ConstantBuffer.h"

class Model;
class Camera;
class Timer;

extern uint32_t g_screenWidth;
extern uint32_t g_screenHeight;
extern float g_imguiWidth;
extern float g_imguiHeight;
extern HWND g_hwnd;

class AppBase
{
  public:
    AppBase();
    virtual ~AppBase();

    int32_t Run();

  public:
    virtual bool Initialize();
    virtual void UpdateGui(const float frameRate);
    virtual void Render();
    virtual void Update(const float dt);

    LRESULT CALLBACK MemberWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  protected:
    void SetViewport(D3D12_VIEWPORT viewport);
    void SetSissorRect(D3D12_RECT rect);
    void UpdateCamera(const float dt);
    void WaitForPreviousFrame();

  private:
    bool InitWindow();
    bool InitD3D();
    virtual bool InitGui();
    void GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter,
                            bool requestHighPerformanceAdapter = false);
    void BuildRootSignature();
    void BuildGlobalConsts();
    void UpdateGlobalConsts(const float dt);
    void SetGlobalConsts(const D3D12_GPU_VIRTUAL_ADDRESS resAddress);
    void BeginRender();
    void EndRender();
    void DestroyPSO();
    void Resize();

    void OnMouse(const float x, const float y);

  public:
    static float GetAspect()
    {
        return ((float)g_screenWidth - g_imguiWidth) / g_screenHeight;
    }

  protected:
    Camera *m_camera                   = nullptr;
    static const uint32_t s_frameCount = 2;
    // Pipeline objects.
    D3D12_VIEWPORT m_viewport                     = {};
    D3D12_RECT m_scissorRect                      = {};
    IDXGISwapChain1 *m_swapChain                  = nullptr;
    ID3D12Device *m_device                        = nullptr;
    ID3D12CommandAllocator *m_commandAllocator    = nullptr;
    ID3D12CommandQueue *m_commandQueue            = nullptr;
    ID3D12DescriptorHeap *m_rtvHeap               = nullptr;
    ID3D12DescriptorHeap *m_dsvHeap               = nullptr;
    ID3D12DescriptorHeap *m_srvHeap               = nullptr;
    ID3D12GraphicsCommandList *m_commandList      = nullptr;
    ID3D12RootSignature *m_rootSignature          = nullptr;
    ID3D12Resource *m_depthStencilBuffer          = nullptr;
    ID3D12Resource *m_renderTargets[s_frameCount] = {
        nullptr,
    };

    GlobalConsts m_globalConstData = {};
    UploadBuffer<GlobalConsts> m_globalConstsBuffer;

    uint32_t m_rtvDescriptorSize = 0;
    uint32_t m_dsvDescriptorSize = 0;

    // Synchronization objects.
    uint32_t m_frameIndex = 0;
    HANDLE m_fenceEvent   = nullptr;
    ID3D12Fence *m_fence  = nullptr;
    uint64_t m_fenceValue = 0;

    HWND m_hwnd          = nullptr;
    bool m_useWarpDevice = false;
    // Gui control
    bool m_isFPV        = false;
    bool m_drawAsNormal = false;
    bool m_isWireFrame  = false;
    bool m_useMSAA      = false;
    bool m_useTexture   = true;
    // Mouse control
    bool m_leftButtonDown       = false;
    bool m_rightButtonDown      = false;
    bool m_leftButtonDragStart  = false;
    bool m_rightButtonDragStart = false;
    float m_mouseX              = 0.0f;
    float m_mouseY              = 0.0f;
    float m_ndcX                = 0.0f;
    float m_ndcY                = 0.0f;

  private:
    Timer *m_timer = nullptr;
    // Key control
    bool m_isKeyDown[256] = {};
};
