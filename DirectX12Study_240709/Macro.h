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

#define CREATE_OBJ(obj, type)                                                                                          \
    obj = new type;                                                                                                    \
    if (!obj)                                                                                                          \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

#define CREATE_MODEL_OBJ(obj)                                                                                          \
    CREATE_OBJ(obj, Model)

#define CREATE_SKINNED_OBJ(obj)                                                                                        \
    CREATE_OBJ(obj, SkinnedMeshModel)