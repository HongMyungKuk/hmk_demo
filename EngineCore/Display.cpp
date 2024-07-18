#include "pch.h"

#include "ColorBuffer.h"
#include "CommandListManager.h"
#include "Display.h"
#include "GraphicsCore.h"

namespace EngineCore
{
extern HWND g_hwnd;
}

#define SWAP_CHAIN_BUFFER_COUNT 2

DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

using namespace Graphics;

namespace Graphics
{
uint32_t g_displayWidth  = 1280;
uint32_t g_displayHeight = 800;
} // namespace Graphics

namespace Display
{
ColorBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];

IDXGISwapChain1 *s_swapChain1 = nullptr;

void Initialize()
{
    assert(s_swapChain1 == nullptr);

    IDXGIFactory6 *factory = nullptr;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount           = SWAP_CHAIN_BUFFER_COUNT;
    swapChainDesc.Width                 = g_displayWidth;
    swapChainDesc.Height                = g_displayHeight;
    swapChainDesc.Format                = SwapChainFormat;
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.Flags                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
    fsSwapChainDesc.Windowed                        = true;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(g_CommandManager.GetCommandQueue(), EngineCore::g_hwnd,
                                                  &swapChainDesc, nullptr, nullptr, &s_swapChain1));

    for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
    {
        ID3D12Resource *displayPlane = nullptr;
        ThrowIfFailed(s_swapChain1->GetBuffer(i, IID_PPV_ARGS(&displayPlane)));
        g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", displayPlane);
        displayPlane = nullptr;
    }

    SAFE_RELEASE(factory);
}

void Shutdown()
{
    if (s_swapChain1)
        s_swapChain1->SetFullscreenState(false, nullptr);
    SAFE_RELEASE(s_swapChain1);

    for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
    {
        g_DisplayPlane[i].Destroy();
    }
}

void Resize(uint32_t width, uint32_t height)
{
}

void Present()
{
    s_swapChain1->Present(1, 0);
}
} // namespace Display