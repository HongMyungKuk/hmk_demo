#include "pch.h"

#include "ModelViewer.h"
#include <iostream>

int main()
{
    ModelViewer modelViewer;

    if (!modelViewer.Initialize())
    {
        return false;
    }

    return modelViewer.Run();
}