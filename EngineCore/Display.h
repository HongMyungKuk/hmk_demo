#pragma once

namespace Graphics
{
extern uint32_t g_displayWidth;
extern uint32_t g_displayHeight;
} // namespace Graphics

namespace Display
{
void Initialize();
void Shutdown();
void Resize(uint32_t width, uint32_t height);
void Present();
} // namespace Display