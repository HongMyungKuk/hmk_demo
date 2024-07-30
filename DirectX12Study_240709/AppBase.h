#pragma once

#include "ColorBuffer.h"
#include "ConstantBuffer.h"
#include "DepthBuffer.h"
#include "DescriptorHeap.h"
#include "EventHandler.h"
#include "PostEffects.h"
#include "PostProcess.h"

class Model;
class Camera;
class Timer;
class ColorBuffer;

extern EventHandler g_EvnetHandler;

namespace Display
{
extern uint32_t g_screenWidth;
extern uint32_t g_screenHeight;
extern float g_imguiWidth;
extern float g_imguiHeight;
} // namespace Display

namespace Graphics
{
extern ID3D12Device *g_Device;
extern ColorBuffer g_DisplayPlane[];

extern DescriptorHeap s_Texture;
extern DescriptorHeap s_Sampler;
extern DescriptorAllocator g_DescriptorAllocator[];
inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count = 1)
{
    return g_DescriptorAllocator[type].Allocate(count);
}
} // namespace Graphics

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
    void UpdateCamera(const float dt);
    void WaitForPreviousFrame();
    void InitCubemap(std::wstring basePath, std::wstring envFilename, std::wstring diffuseFilename,
                     std::wstring specularFilename);
    virtual void InitLights();
    virtual void UpdateLights();

  private:
    bool InitWindow();
    bool InitD3D();
    virtual bool InitGui();
    void GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter,
                            bool requestHighPerformanceAdapter = false);
    void InitGlobalConsts();
    void InitSRVDesriptorHeap();
    void UpdateGlobalConsts(const float dt);
    void RenderDepthOnlyPass();
    void RenderOpaqueObject();
    void RenderDepthMapViewport();
    void RenderPostProcess();
    void DestroyPSO();
    void CreateBuffers();
    void Resize();

    void OnMouse(const float x, const float y);

  public:
    static float GetAspect()
    {
        return ((float)Display::g_screenWidth - Display::g_imguiWidth) / Display::g_screenHeight;
    }

  protected:
    Camera *m_camera                   = nullptr;
    static const uint32_t s_frameCount = 2;
    // Pipeline objects.
    IDXGISwapChain1 *m_swapChain               = nullptr;
    ID3D12Device *m_device                     = nullptr;
    ID3D12CommandAllocator *m_commandAllocator = nullptr;
    ID3D12CommandQueue *m_commandQueue         = nullptr;
    ID3D12GraphicsCommandList *m_commandList   = nullptr;

    GlobalConsts m_globalConstsData             = {};
    GlobalConsts m_shadowConstsData[MAX_LIGHTS] = {};
    UploadBuffer<GlobalConsts> m_globalConstsBuffer;
    UploadBuffer<GlobalConsts> m_shadowConstBuffers;

    ColorBuffer m_floatBuffer[2];
    ColorBuffer m_resolvedBuffer[2];
    DepthBuffer m_depthBuffer;
    DepthBuffer m_depthOnlyBuffer;
    DepthBuffer m_shadowMap[MAX_LIGHTS];
    DescriptorHeap m_imguiInitHeap;
    DescriptorHandle m_cubeMapHandle[3];
    ID3D12Resource* m_cubeMapResource[3];
    int m_cubeMapType = 2;

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
    bool m_isKeyDown[256]                      = {};

  protected:
    // Object list.
    Light m_light[3]                    = {};
    std::vector<Model *> m_lightSpheres = {};
    std::vector<Model *> m_opaqueList   = {};
    Model *m_skybox                     = nullptr;
    Model *m_depthMap                   = nullptr;

    PostEffects m_postEffects;
    PostProcess m_postProcess;
    float m_gammaFactor    = 2.2f;
    float m_exposureFactor = 1.0f;

    float m_metalness = 0.0f;
    float m_roughness = 0.0f;
};
