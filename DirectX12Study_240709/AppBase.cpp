#include "pch.h"

#include "AppBase.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "Input.h"
#include "Model.h"
#include "Timer.h"

AppBase *g_appBase         = nullptr;
uint32_t g_screenWidth     = 1200;
uint32_t g_screenHeight    = 800;
extern float g_imguiWidth  = 0.0f;
extern float g_imguiHeight = 0.0f;
HWND g_hwnd                = nullptr;

ID3D12DescriptorHeap *m_desciptorHeap = nullptr;
uint32_t g_descCnt                    = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

AppBase::AppBase()
{
    g_appBase = this;
}

AppBase::~AppBase()
{
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);

    Graphics::DestroyGraphicsCommon();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    SAFE_DELETE(m_camera);
    SAFE_RELEASE(m_rootSignature)
    SAFE_DELETE(m_timer);
    SAFE_RELEASE(m_fence);
    SAFE_RELEASE(m_commandList);
    SAFE_RELEASE(m_commandAllocator);
    SAFE_RELEASE(m_depthStencilBuffer);
    for (uint8_t i = 0; i < s_frameCount; i++)
        SAFE_RELEASE(m_renderTargets[i]);
    SAFE_RELEASE(m_srvHeap);
    SAFE_RELEASE(m_dsvHeap);
    SAFE_RELEASE(m_rtvHeap);
    SAFE_RELEASE(m_swapChain);
    SAFE_RELEASE(m_commandQueue);
    SAFE_RELEASE(m_device);
}

bool AppBase::Initialize()
{
    if (!InitWindow())
    {
        return false;
    }
    if (!InitD3D())
    {
        return false;
    }
    if (!InitGui())
    {
        return false;
    }

    // Init Timer
    CREATE_OBJ(m_timer, Timer);
    m_timer->Initialize();

    // Mouse & Keyboard input initialize.
    GameInput::Initialize();

    this->BuildRootSignature();

    D3DUtils::CreateDscriptor(m_device, 300, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                              D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, &m_desciptorHeap);
    g_descCnt++;

    this->BuildGlobalConsts();

    // Init graphics common.
    Graphics::InitGraphicsCommon(m_device, m_rootSignature);

    return true;
}
void AppBase::Update(const float dt)
{
    m_timer->Update();

    GameInput::Update(dt);

    UpdateGlobalConsts(dt);
}

void AppBase::Render()
{
}

int32_t AppBase::Run()
{
    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (m_isKeyDown[VK_ESCAPE])
                PostQuitMessage(0);

            // Start the Dear ImGui frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGuiIO &io = ImGui::GetIO();
            (void)io;

            this->UpdateGui(io.Framerate);

            ImGui::Render();

            this->Update(io.Framerate);

            AppBase::BeginRender();
            this->Render();
            m_commandList->SetDescriptorHeaps(1, &m_srvHeap);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList);
            AppBase::EndRender();
        }
    }
    // Return this part of the WM_QUIT message to Windows.
    return static_cast<int32_t>(msg.wParam);
}

bool AppBase::InitWindow()
{
    // Initialize the window class.
    WNDCLASSEX windowClass    = {};
    windowClass.cbSize        = sizeof(WNDCLASSEX);
    windowClass.style         = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc   = WndProc;
    windowClass.hInstance     = GetModuleHandle(nullptr);
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DX12Study";
    RegisterClassEx(&windowClass);

    RECT windowRect = {0, 0, static_cast<LONG>(g_screenWidth), static_cast<LONG>(g_screenHeight)};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    g_hwnd = m_hwnd =
        CreateWindow(windowClass.lpszClassName, windowClass.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                     CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                     nullptr, // We have no parent window.
                     nullptr, // We aren't using menus.
                     windowClass.hInstance, nullptr);
    ShowWindow(m_hwnd, SW_SHOW);

    return true;
}

bool AppBase::InitD3D()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ID3D12Debug *debugController = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            SAFE_RELEASE(debugController);
        }
    }
#endif
    IDXGIFactory6 *factory = nullptr;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        IDXGIAdapter *warpAdapter = nullptr;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
        SAFE_RELEASE(warpAdapter);
    }
    else
    {
        IDXGIAdapter1 *hardwareAdapter = nullptr;
        GetHardwareAdapter(factory, &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
        SAFE_RELEASE(hardwareAdapter);
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    // DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    // swapChainDesc.BufferCount                        = s_frameCount;
    // swapChainDesc.BufferDesc.Width                   = g_screenWidth;
    // swapChainDesc.BufferDesc.Height                  = g_screenHeight;
    // swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    // swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    // swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    // swapChainDesc.OutputWindow                       = m_hwnd;
    // swapChainDesc.Windowed                           = true;
    // swapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    // swapChainDesc.SampleDesc.Count                   = 1;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount           = s_frameCount;
    swapChainDesc.Width                 = g_screenWidth;
    swapChainDesc.Height                = g_screenHeight;
    swapChainDesc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.Flags                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain1 *swapChain = nullptr;
    ThrowIfFailed(
        factory->CreateSwapChainForHwnd(m_commandQueue, m_hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
    m_swapChain = swapChain;
    swapChain   = nullptr;

    // Create descriptor heaps.
    {
        D3DUtils::CreateDscriptor(m_device, s_frameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, &m_rtvHeap);
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a render target view (DSV) descriptor heap.
        D3DUtils::CreateDscriptor(m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                                  &m_dsvHeap);
        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Describe and create a render target view (SRV) descriptor heap.
        D3DUtils::CreateDscriptor(m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, &m_srvHeap);
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr,
                                              IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Create synchronization objects.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX       = 0;
    viewport.TopLeftY       = 0;
    viewport.MinDepth       = 0.0f;
    viewport.MaxDepth       = 1.0f;
    viewport.Width          = (FLOAT)g_screenWidth;
    viewport.Height         = (FLOAT)g_screenHeight;
    this->SetViewport(viewport);

    D3D12_RECT sissorRect = {};
    sissorRect.left       = 0;
    sissorRect.top        = 0;
    sissorRect.right      = g_screenWidth;
    sissorRect.bottom     = g_screenHeight;
    this->SetSissorRect(sissorRect);

    this->Resize();

    SAFE_RELEASE(factory);

    return true;
}

bool AppBase::InitGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX12_Init(m_device, 3, DXGI_FORMAT_R8G8B8A8_UNORM, m_srvHeap,
                        m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
                        m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    return true;
}

void AppBase::GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    IDXGIAdapter1 *adapter = nullptr;

    IDXGIFactory6 *factory6 = nullptr;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                 adapterIndex,
                 requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                                       : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                 IID_PPV_ARGS(&adapter)));
             ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter;
    adapter    = nullptr;

    SAFE_RELEASE(factory6);
}

void AppBase::BuildRootSignature()
{
    // Create root signature.
    CD3DX12_DESCRIPTOR_RANGE rangeObj[1] = {};
    rangeObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 300, 0); // t0: envTex, t1 ~ 299 : map texture
    //rangeObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    CD3DX12_ROOT_PARAMETER rootParameters[5] = {};
    rootParameters[0].InitAsConstantBufferView(0); // b0 : Global Consts
    rootParameters[1].InitAsConstantBufferView(1); // b1 : Mesh Consts
    rootParameters[2].InitAsConstantBufferView(2); // b2 : material Consts
    rootParameters[3].InitAsDescriptorTable(_countof(rangeObj), rangeObj, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[4].InitAsConstantBufferView(3); // b3 : material Consts

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias                = 0;
    sampler.MaxAnisotropy             = 0;
    sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor               = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD                    = 0.0f;
    sampler.MaxLOD                    = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister            = 0;
    sampler.RegisterSpace             = 0;
    sampler.ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

    ID3DBlob *signature = nullptr;
    ID3DBlob *error     = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                IID_PPV_ARGS(&m_rootSignature)));
}

void AppBase::BuildGlobalConsts()
{
    m_globalConstsBuffer.Initialize(m_device, 1);
}

void AppBase::UpdateGlobalConsts(const float dt)
{
    auto eyePos  = m_camera->GetPosition();
    auto viewRow = m_camera->GetViewMatrix();
    auto projRow = m_camera->GetProjectionMatrix();

    m_globalConstData.eyeWorld    = eyePos;
    m_globalConstData.view        = viewRow.Transpose();
    m_globalConstData.viewInv     = m_globalConstData.view.Invert();
    m_globalConstData.proj        = projRow.Transpose();
    m_globalConstData.projInv     = m_globalConstData.proj.Invert();
    m_globalConstData.viewProjInv = (viewRow * projRow).Invert().Transpose();

    m_globalConstsBuffer.Upload(0, &m_globalConstData);
}

void AppBase::UpdateCamera(const float dt)
{
    if (GameInput::IsPressed(GameInput::kKey_w))
    {
        m_camera->MoveFront(dt);
    }
    if (GameInput::IsPressed(GameInput::kKey_s))
    {
        m_camera->MoveBack(dt);
    }
    if (GameInput::IsPressed(GameInput::kKey_d))
    {
        m_camera->MoveRight(dt);
    }
    if (GameInput::IsPressed(GameInput::kKey_a))
    {
        m_camera->MoveLeft(dt);
    }
    if (GameInput::IsPressed(GameInput::kKey_q))
    {
        m_camera->MoveUp(dt);
    }
    if (GameInput::IsPressed(GameInput::kKey_e))
    {
        m_camera->MoveDown(dt);
    }
}

void AppBase::UpdateGui(const float frameRate)
{
}

void AppBase::SetGlobalConsts(const D3D12_GPU_VIRTUAL_ADDRESS resAddress)
{
    m_commandList->SetGraphicsRootConstantBufferView(0, resAddress);
}

void AppBase::BeginRender()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_commandList->SetGraphicsRootSignature(m_rootSignature);
    //  Global consts
    this->SetGlobalConsts(m_globalConstsBuffer.GetResource()->GetGPUVirtualAddress());

    // TODO!!
    // 힙을 한번에 만들어 놓고 쓴다.
    ID3D12DescriptorHeap *descHeaps[] = {m_desciptorHeap};
    m_commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(3, m_desciptorHeap->GetGPUDescriptorHandleForHeapStart());

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_PRESENT,
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, false, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    // Record commands.
    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
                                         D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void AppBase::EndRender()
{
    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                            D3D12_RESOURCE_STATE_PRESENT));
    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();

    m_frameIndex = (m_frameIndex + 1) % s_frameCount;
}

void AppBase::DestroyPSO()
{
    SAFE_RELEASE(Graphics::defaultWirePSO);
    SAFE_RELEASE(Graphics::defaultSolidPSO);
}

void AppBase::OnMouse(const float x, const float y)
{
    auto newScreenWidth = float(g_screenWidth - g_imguiWidth);
    auto newSreenHeight = float(g_screenHeight);

    m_mouseX = std::clamp(x, 0.0f, newScreenWidth);
    m_mouseY = std::clamp(y, 0.0f, newSreenHeight);

    m_ndcX = m_mouseX / newScreenWidth * 2.0f - 1.0f;
    m_ndcY = -(m_mouseY / newSreenHeight * 2.0f - 1.0f);

    m_ndcX = std::clamp(m_ndcX, -1.0f, 1.0f);
    m_ndcY = std::clamp(m_ndcY, -1.0f, 1.0f);

    if (m_isFPV)
    {
        m_camera->MouseUpdate(m_ndcX, m_ndcY);
    }
}

LRESULT AppBase::MemberWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_CREATE:
        return 0;

    case WM_SIZE: {

        RECT rect = {};
        GetClientRect(hWnd, &rect);
        g_screenWidth  = uint32_t(rect.right - rect.left);
        g_screenHeight = uint32_t(rect.bottom - rect.top);

        if (m_swapChain)
        {
            this->Resize();

            D3D12_VIEWPORT viewport = {};
            viewport.TopLeftX       = 0;
            viewport.TopLeftY       = 0;
            viewport.MinDepth       = 0.0f;
            viewport.MaxDepth       = 1.0f;
            viewport.Width          = (FLOAT)g_screenWidth;
            viewport.Height         = (FLOAT)g_screenHeight;
            this->SetViewport(viewport);

            D3D12_RECT rect = {};
            rect.left       = 0;
            rect.top        = 0;
            rect.right      = g_screenWidth;
            rect.bottom     = g_screenHeight;
            this->SetSissorRect(rect);
        }
    }
    break;
    case WM_KEYDOWN:
        if (wParam == 70)
            m_isFPV = !m_isFPV;
        m_isKeyDown[wParam] = true;
        break;
    case WM_KEYUP:
        m_isKeyDown[wParam] = false;
        break;
    case WM_MOUSEMOVE:
        // std::cout << HIWORD(lParam) << " " << LOWORD(lParam) << std::endl;
        OnMouse(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        m_leftButtonDown      = true;
        m_leftButtonDragStart = true;
        break;
    case WM_LBUTTONUP:
        m_leftButtonDown = false;
        break;
    case WM_RBUTTONDOWN:
        m_rightButtonDown      = true;
        m_rightButtonDragStart = true;
        break;
    case WM_RBUTTONUP:
        m_rightButtonDown = false;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void AppBase::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence, fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void AppBase::InitCubemap(std::wstring basePath, std::wstring envFilename)
{
    ResourceUploadBatch resourceUpload(m_device);

    resourceUpload.Begin();

    bool isCubemap = false;
    ThrowIfFailed(CreateDDSTextureFromFile(m_device, resourceUpload, (basePath + envFilename).c_str(), &m_envTexture,
                                           false, 0, nullptr, &isCubemap));

    // Upload the resources to the GPU.
    auto uploadResourcesFinished = resourceUpload.End(m_commandQueue);

    // Wait for the upload thread to terminate
    uploadResourcesFinished.wait();

    auto desc = m_envTexture->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip     = 0;
    srvDesc.TextureCube.MipLevels           = m_envTexture->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format                          = m_envTexture->GetDesc().Format;

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_desciptorHeap->GetCPUDescriptorHandleForHeapStart());
    m_device->CreateShaderResourceView(m_envTexture, &srvDesc, srvHandle);
}

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return g_appBase->MemberWndProc(hWnd, message, wParam, lParam);
}

void AppBase::SetViewport(D3D12_VIEWPORT viewport)
{
    m_viewport = viewport;
}

void AppBase::SetSissorRect(D3D12_RECT rect)
{
    m_scissorRect = rect;
}

void AppBase::Resize()
{
    m_commandList->Reset(m_commandAllocator, nullptr);
    // Create frame resources.
    {
        // Reset a RTV for each frame.
        if (m_renderTargets[0])
        {
            for (UINT n = 0; n < s_frameCount; n++)
            {
                m_renderTargets[n]->Release();
                m_renderTargets[n] = nullptr;
            }
        }
        if (m_depthStencilBuffer)
        {
            m_depthStencilBuffer->Release();
            m_depthStencilBuffer = nullptr;
        }
        // swap chain resize.
        if (m_swapChain)
        {
            ThrowIfFailed(m_swapChain->ResizeBuffers(s_frameCount, g_screenWidth, g_screenHeight,
                                                     DXGI_FORMAT_R8G8B8A8_UNORM,
                                                     DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
        }

        m_frameIndex = 0;

        // Create a RTV for each frame.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT n = 0; n < s_frameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }

        D3D12_RESOURCE_DESC depthStencilDesc = {};
        depthStencilDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment           = 0;
        depthStencilDesc.Width               = (UINT64)g_screenWidth;
        depthStencilDesc.Height              = (UINT64)g_screenHeight;
        depthStencilDesc.DepthOrArraySize    = 1;
        depthStencilDesc.MipLevels           = 1;
        depthStencilDesc.Format              = DXGI_FORMAT_R24G8_TYPELESS;
        depthStencilDesc.SampleDesc.Count    = 1;
        depthStencilDesc.SampleDesc.Quality  = 0;
        depthStencilDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags               = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        optClear.DepthStencil.Depth   = 1.0f;
        optClear.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_depthStencilBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.Texture2D.MipSlice = 0;
        m_device->CreateDepthStencilView(m_depthStencilBuffer, &dsvDesc,
                                         m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        // Transition the resource from its initial state to be used as a depth buffer.
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer,
                                                                                D3D12_RESOURCE_STATE_COMMON,
                                                                                D3D12_RESOURCE_STATE_DEPTH_WRITE));
        // For texture loading.
        ThrowIfFailed(m_commandList->Close());
        ID3D12CommandList *ppCommandLists[] = {m_commandList};
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        WaitForPreviousFrame();
    }
}