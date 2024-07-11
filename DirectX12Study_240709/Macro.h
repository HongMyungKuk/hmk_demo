#pragma once

#define SAFE_RELEASE(ptr)                                                                                              \
    if (ptr)                                                                                                           \
    {                                                                                                                  \
        ptr->Release();                                                                                                \
        ptr = nullptr;                                                                                                 \
    }

#define SAFE_DELETE(ptr)                                                                                               \
    if (ptr)                                                                                                           \
    {                                                                                                                  \
        delete ptr;                                                                                                    \
        ptr = nullptr;                                                                                                 \
    }

#define SAFE_ARR_DELETE(ptr)                                                                                           \
    if (ptr)                                                                                                           \
    {                                                                                                                  \
        delete[] ptr;                                                                                                  \
        ptr = nullptr;                                                                                                 \
    }

#define CREATE_MODEL_OBJ(obj)                                                                                          \
    obj = new Model;                                                                                                   \
    if (!obj)                                                                                                          \
    {                                                                                                                  \
        return false;                                                                                                  \
    }