#pragma once

#include "pch.h"

namespace EngineCore
{
class EngineApp
{
  public:
    virtual void Startup(void) = 0;
    virtual void Cleanup(void) = 0;

    virtual bool IsDone(void);

    virtual void Update(float dt) = 0;

    virtual void RenderScene(void) = 0;

    virtual void RenderUI(class GraphicsContext &){};

    virtual bool RequiresRaytracingSupport() const
    {
        return false;
    }
};
} // namespace EngineCore

namespace EngineCore
{
int RunApplication(EngineApp &app, const wchar_t *className);
}

#define CREATE_APPLICATION(app_class)                                                                                  \
    int main()                                                                                                         \
    {                                                                                                                  \
        return EngineCore::RunApplication(app_class(), L#app_class);                                                   \
    }