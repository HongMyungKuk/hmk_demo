#include "pch.h"
#include "DescriptorHeap.h"

DescriptorHeap::~DescriptorHeap()
{
	SAFE_RELEASE(m_descriptorHeap);
}
