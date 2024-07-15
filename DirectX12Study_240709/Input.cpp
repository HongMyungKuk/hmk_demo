#include "pch.h"

#include "AppBase.h"
#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace
{
bool s_buttons[2][GameInput::kNumDigitalInputs]    = {};
float s_holdDuration[GameInput::kNumDigitalInputs] = {};
float s_Analogs[GameInput::kNumAnalogInputs]       = {};
float s_AnalogsTC[GameInput::kNumAnalogInputs]     = {};

IDirectInput8A *s_di             = nullptr;
IDirectInputDevice8A *s_keyboard = nullptr;
IDirectInputDevice8A *s_mouse    = nullptr;

DIMOUSESTATE2 s_mouseState                        = {};
unsigned char s_keybuffer[256]                    = {};
unsigned char s_dxKeyMapping[GameInput::kNumKeys] = {};

void KbmBuildKeyMapping()
{
    s_dxKeyMapping[GameInput::kKey_escape]      = 1;
    s_dxKeyMapping[GameInput::kKey_1]           = 2;
    s_dxKeyMapping[GameInput::kKey_2]           = 3;
    s_dxKeyMapping[GameInput::kKey_3]           = 4;
    s_dxKeyMapping[GameInput::kKey_4]           = 5;
    s_dxKeyMapping[GameInput::kKey_5]           = 6;
    s_dxKeyMapping[GameInput::kKey_6]           = 7;
    s_dxKeyMapping[GameInput::kKey_7]           = 8;
    s_dxKeyMapping[GameInput::kKey_8]           = 9;
    s_dxKeyMapping[GameInput::kKey_9]           = 10;
    s_dxKeyMapping[GameInput::kKey_0]           = 11;
    s_dxKeyMapping[GameInput::kKey_minus]       = 12;
    s_dxKeyMapping[GameInput::kKey_equals]      = 13;
    s_dxKeyMapping[GameInput::kKey_back]        = 14;
    s_dxKeyMapping[GameInput::kKey_tab]         = 15;
    s_dxKeyMapping[GameInput::kKey_q]           = 16;
    s_dxKeyMapping[GameInput::kKey_w]           = 17;
    s_dxKeyMapping[GameInput::kKey_e]           = 18;
    s_dxKeyMapping[GameInput::kKey_r]           = 19;
    s_dxKeyMapping[GameInput::kKey_t]           = 20;
    s_dxKeyMapping[GameInput::kKey_y]           = 21;
    s_dxKeyMapping[GameInput::kKey_u]           = 22;
    s_dxKeyMapping[GameInput::kKey_i]           = 23;
    s_dxKeyMapping[GameInput::kKey_o]           = 24;
    s_dxKeyMapping[GameInput::kKey_p]           = 25;
    s_dxKeyMapping[GameInput::kKey_lbracket]    = 26;
    s_dxKeyMapping[GameInput::kKey_rbracket]    = 27;
    s_dxKeyMapping[GameInput::kKey_return]      = 28;
    s_dxKeyMapping[GameInput::kKey_lcontrol]    = 29;
    s_dxKeyMapping[GameInput::kKey_a]           = 30;
    s_dxKeyMapping[GameInput::kKey_s]           = 31;
    s_dxKeyMapping[GameInput::kKey_d]           = 32;
    s_dxKeyMapping[GameInput::kKey_f]           = 33;
    s_dxKeyMapping[GameInput::kKey_g]           = 34;
    s_dxKeyMapping[GameInput::kKey_h]           = 35;
    s_dxKeyMapping[GameInput::kKey_j]           = 36;
    s_dxKeyMapping[GameInput::kKey_k]           = 37;
    s_dxKeyMapping[GameInput::kKey_l]           = 38;
    s_dxKeyMapping[GameInput::kKey_semicolon]   = 39;
    s_dxKeyMapping[GameInput::kKey_apostrophe]  = 40;
    s_dxKeyMapping[GameInput::kKey_grave]       = 41;
    s_dxKeyMapping[GameInput::kKey_lshift]      = 42;
    s_dxKeyMapping[GameInput::kKey_backslash]   = 43;
    s_dxKeyMapping[GameInput::kKey_z]           = 44;
    s_dxKeyMapping[GameInput::kKey_x]           = 45;
    s_dxKeyMapping[GameInput::kKey_c]           = 46;
    s_dxKeyMapping[GameInput::kKey_v]           = 47;
    s_dxKeyMapping[GameInput::kKey_b]           = 48;
    s_dxKeyMapping[GameInput::kKey_n]           = 49;
    s_dxKeyMapping[GameInput::kKey_m]           = 50;
    s_dxKeyMapping[GameInput::kKey_comma]       = 51;
    s_dxKeyMapping[GameInput::kKey_period]      = 52;
    s_dxKeyMapping[GameInput::kKey_slash]       = 53;
    s_dxKeyMapping[GameInput::kKey_rshift]      = 54;
    s_dxKeyMapping[GameInput::kKey_multiply]    = 55;
    s_dxKeyMapping[GameInput::kKey_lalt]        = 56;
    s_dxKeyMapping[GameInput::kKey_space]       = 57;
    s_dxKeyMapping[GameInput::kKey_capital]     = 58;
    s_dxKeyMapping[GameInput::kKey_f1]          = 59;
    s_dxKeyMapping[GameInput::kKey_f2]          = 60;
    s_dxKeyMapping[GameInput::kKey_f3]          = 61;
    s_dxKeyMapping[GameInput::kKey_f4]          = 62;
    s_dxKeyMapping[GameInput::kKey_f5]          = 63;
    s_dxKeyMapping[GameInput::kKey_f6]          = 64;
    s_dxKeyMapping[GameInput::kKey_f7]          = 65;
    s_dxKeyMapping[GameInput::kKey_f8]          = 66;
    s_dxKeyMapping[GameInput::kKey_f9]          = 67;
    s_dxKeyMapping[GameInput::kKey_f10]         = 68;
    s_dxKeyMapping[GameInput::kKey_numlock]     = 69;
    s_dxKeyMapping[GameInput::kKey_scroll]      = 70;
    s_dxKeyMapping[GameInput::kKey_numpad7]     = 71;
    s_dxKeyMapping[GameInput::kKey_numpad8]     = 72;
    s_dxKeyMapping[GameInput::kKey_numpad9]     = 73;
    s_dxKeyMapping[GameInput::kKey_subtract]    = 74;
    s_dxKeyMapping[GameInput::kKey_numpad4]     = 75;
    s_dxKeyMapping[GameInput::kKey_numpad5]     = 76;
    s_dxKeyMapping[GameInput::kKey_numpad6]     = 77;
    s_dxKeyMapping[GameInput::kKey_add]         = 78;
    s_dxKeyMapping[GameInput::kKey_numpad1]     = 79;
    s_dxKeyMapping[GameInput::kKey_numpad2]     = 80;
    s_dxKeyMapping[GameInput::kKey_numpad3]     = 81;
    s_dxKeyMapping[GameInput::kKey_numpad0]     = 82;
    s_dxKeyMapping[GameInput::kKey_decimal]     = 83;
    s_dxKeyMapping[GameInput::kKey_f11]         = 87;
    s_dxKeyMapping[GameInput::kKey_f12]         = 88;
    s_dxKeyMapping[GameInput::kKey_numpadenter] = 156;
    s_dxKeyMapping[GameInput::kKey_rcontrol]    = 157;
    s_dxKeyMapping[GameInput::kKey_divide]      = 181;
    s_dxKeyMapping[GameInput::kKey_sysrq]       = 183;
    s_dxKeyMapping[GameInput::kKey_ralt]        = 184;
    s_dxKeyMapping[GameInput::kKey_pause]       = 197;
    s_dxKeyMapping[GameInput::kKey_home]        = 199;
    s_dxKeyMapping[GameInput::kKey_up]          = 200;
    s_dxKeyMapping[GameInput::kKey_pgup]        = 201;
    s_dxKeyMapping[GameInput::kKey_left]        = 203;
    s_dxKeyMapping[GameInput::kKey_right]       = 205;
    s_dxKeyMapping[GameInput::kKey_end]         = 207;
    s_dxKeyMapping[GameInput::kKey_down]        = 208;
    s_dxKeyMapping[GameInput::kKey_pgdn]        = 209;
    s_dxKeyMapping[GameInput::kKey_insert]      = 210;
    s_dxKeyMapping[GameInput::kKey_delete]      = 211;
    s_dxKeyMapping[GameInput::kKey_lwin]        = 219;
    s_dxKeyMapping[GameInput::kKey_rwin]        = 220;
    s_dxKeyMapping[GameInput::kKey_apps]        = 221;
}

void KbmZeroInputs()
{
    memset(&s_mouseState, 0, sizeof(DIMOUSESTATE2));
    memset(s_keybuffer, 0, sizeof(s_keybuffer));
}

void KbmInitialize()
{
    KbmBuildKeyMapping();

    ThrowIfFailed(
        DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&s_di, nullptr));

    ThrowIfFailed(s_di->CreateDevice(GUID_SysKeyboard, &s_keyboard, nullptr));
    ThrowIfFailed(s_keyboard->SetDataFormat(&c_dfDIKeyboard));
    ThrowIfFailed(s_keyboard->SetCooperativeLevel(g_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = 10;
    ThrowIfFailed(s_keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph));

    //ThrowIfFailed(s_di->CreateDevice(GUID_SysMouse, &s_mouse, nullptr));
    //ThrowIfFailed(s_mouse->SetDataFormat(&c_dfDIMouse2));
    //ThrowIfFailed(s_mouse->SetCooperativeLevel(g_hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE));

    KbmZeroInputs();
}

void KbmUpdate()
{
    HWND foreground = GetForegroundWindow();
    bool visible    = IsWindowVisible(foreground) != 0;

    if (foreground != g_hwnd || !visible) // wouldn't be able to acquire
    {
        KbmZeroInputs();
    }
    else
    {
        //s_mouse->Acquire();
        //s_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_mouseState);
        s_keyboard->Acquire();
        s_keyboard->GetDeviceState(sizeof(s_keybuffer), s_keybuffer);
    }
}
} // namespace

namespace GameInput
{
void Initialize()
{
    ZeroMemory(s_buttons, sizeof(s_buttons));
    ZeroMemory(s_Analogs, sizeof(s_Analogs));

    KbmInitialize();
}

void Destroy()
{
    if (s_keyboard)
    {
        s_keyboard->Unacquire();
        SAFE_RELEASE(s_keyboard);
    }
    if (s_mouse)
    {
        s_mouse->Unacquire();
        SAFE_RELEASE(s_mouse);
    }
    if (s_di)
    {
        SAFE_RELEASE(s_di);
    }
}

void Update(const float dt)
{
    memcpy(s_buttons[1], s_buttons[0], sizeof(s_buttons[0]));
    memset(s_buttons[0], 0, sizeof(s_buttons[0]));

    KbmUpdate();

    for (uint32_t i = 0; i < kNumKeys; ++i)
    {
        s_buttons[0][i] = (s_keybuffer[s_dxKeyMapping[i]] & 0x80) != 0;
    }

    for (uint32_t i = 0; i < 8; ++i)
    {
        if (s_mouseState.rgbButtons[i] > 0)
            s_buttons[0][kMouse0 + i] = true;
    }

    s_Analogs[kAnalogMouseX] = (float)s_mouseState.lX * .0018f;
    s_Analogs[kAnalogMouseY] = (float)s_mouseState.lY * -.0018f;

    if (s_mouseState.lZ > 0)
        s_Analogs[kAnalogMouseScroll] = 1.0f;
    else if (s_mouseState.lZ < 0)
        s_Analogs[kAnalogMouseScroll] = -1.0f;

    // Update time duration for buttons pressed
    for (uint32_t i = 0; i < kNumDigitalInputs; ++i)
    {
        if (s_buttons[0][i])
        {
            if (!s_buttons[1][i])
                s_holdDuration[i] = 0.0f;
            else
                s_holdDuration[i] += dt;
        }
    }

    for (uint32_t i = 0; i < kNumAnalogInputs; ++i)
    {
        s_AnalogsTC[i] = s_Analogs[i] * dt;
    }
}

bool IsAnyPressed(void)
{
    return s_buttons[0];
}

bool IsPressed(DigitalInput di)
{
    return s_buttons[0][di];
}

bool IsFirstPressed(DigitalInput di)
{
    return s_buttons[0][di] && !s_buttons[1][di];
}

bool IsReleased(DigitalInput di)
{
    return !s_buttons[0][di];
}

bool IsFirstReleased(DigitalInput di)
{
    return !s_buttons[0][di] && s_buttons[1][di];
}

float GetDurationPressed(DigitalInput di)
{
    return s_holdDuration[di];
}

float GetAnalogInput(AnalogInput ai)
{
    return s_Analogs[ai];
}

float GetTimeCollectedAnalogInput(AnalogInput ai)
{
    return s_AnalogsTC[ai];
}
} // namespace GameInput