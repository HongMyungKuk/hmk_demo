#include "pch.h"

#include "ModelViewer.h"
#include "MapTool.h"
#include <iostream>

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
        app = new MapTool;
    }

    if (!app->Initialize())
    {
        return -1;
    }

    if (app->Run())
    {
        return -1;
    }

    delete app;

    return 0;
}