#include "pch.h"

#include "ModelViewer.h"
#include <iostream>

int main()
{
    AppBase app;

    if (!app.Initialize())
    {
        return false;
    }

    return app.Run();
}