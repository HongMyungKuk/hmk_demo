#pragma once

class ColorBuffer;

namespace Graphics
{
extern uint32_t g_displayWidth;
extern uint32_t g_displayHeight;
} // namespace Graphics

namespace Display
{
extern ColorBuffer g_DisplayPlane[];

void Initialize();
void Shutdown();
void Resize(uint32_t width, uint32_t height);
void Present();
} // namespace Display