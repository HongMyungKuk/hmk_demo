#include "pch.h"

#include "ModelViewer.h"
#include "MapTool.h"
#include "Engine.h"
#include "CollisionSample.h"
#include "ComputeShader.h"

int main(int argc, char *argv[])
{
    AppBase* app = nullptr;

    if (argc < 2)
    {
        return -1;
    }

    int n = std::stoi(argv[1]);

    switch (n)
    {
    case 0:
        app = new ModelViewer;
        break;
    case 1:
        //app = new MapTool;
        break;
    case 2:
        app = new CollisionSample;
        break;
    case 3:
        app = new Engine;
        break;
    case 4:
        app = new ComputeShader;
        break;
    }

    if (!app->Initialize())
    {
        return -1;
    }

    if (!app->Run())
    {
        return -1;
    }

    delete app;

    return 0;
}