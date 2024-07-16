// Core.cpp : Defines the functions for the static library.
//

#include "pch.h"

#include "EngineCore.h"
#include "Input.h"

uint32_t g_displayWidth  = 1280;
uint32_t g_displayHeight = 800;

namespace EngineCore
{
HWND g_hwnd = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void InitializeApplication(EngineApp& app)
{
    GameInput::Initialize();

    app.Startup();
}

void TerminateApplication(EngineApp& app)
{
    app.Cleanup();

    GameInput::Destroy();
}

bool UpdateApplication(EngineApp& app)
{
    float dt = 0.01f;

    GameInput::Update(dt);

    app.Update(dt);
    app.RenderScene();

    return app.IsDone();
}

int RunApplication(EngineApp &app, const wchar_t *className)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = GetModuleHandle(nullptr);
    wcex.hIcon         = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = nullptr;
    wcex.lpszClassName = className;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    assert(0 != RegisterClassEx(&wcex));

    // Create window
    RECT rc = {0, 0, (LONG)g_displayWidth, (LONG)g_displayHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    g_hwnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
                          rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);

    assert(g_hwnd != 0);

    InitializeApplication(app);

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);

    do
    {
        MSG msg   = {};
        bool done = false;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                done = true;
        }

        if (done)
            break;
    } while (UpdateApplication(app)); // Returns false to quit loop

    return 0;
}

bool EngineApp::IsDone()
{
    return !GameInput::IsFirstPressed(GameInput::kKey_escape);
}

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}
} // namespace EngineCore