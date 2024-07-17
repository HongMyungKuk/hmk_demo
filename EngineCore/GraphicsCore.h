#pragma once

#include "DesciptorHeap.h"

class CommandListManager;
class ContextManager;

namespace Graphics
{
extern ID3D12Device *g_Device;
extern CommandListManager g_CommandManager;
extern ContextManager g_ContextManager;
extern DescriptorAllocator g_DesciptorAllocator[];

void Initialize(bool requireDXRSupport = false);
void Shutdown();

inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count = 1)
{
    return g_DesciptorAllocator[type].Allocate(count);
}
}
