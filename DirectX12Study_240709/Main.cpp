#include "ModelViewer.h"
#include <iostream>

int main()
{
    ModelViewer viewer;

    if (!viewer.Initialize())
    {
        return false;
    }

    return viewer.Run();
}