#pragma once
#include "AppBase.h"

class ComputeShader :
    public AppBase
{
public:
    ComputeShader();
    virtual ~ComputeShader();

    virtual bool Initialize();
    virtual void UpdateGui(const float frameRate);
    virtual void Render();
    virtual void Update(const float dt);

private:
    ColorBuffer m_uavBuffer;
    
};

