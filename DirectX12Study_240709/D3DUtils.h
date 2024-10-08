#pragma once

#include <stdexcept>

using namespace DirectX;

extern void ReadImage(uint8_t **image, const std::string &filename, int &w, int &h, int &c);

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
  public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr)
    {
    }
    HRESULT Error() const
    {
        return m_hr;
    }

  private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

class DescriptorHandle;

class D3DUtils
{
  public:
    template <typename T_CONST>
    static void CreateConstantBuffer(ID3D12Device *device, ID3D12Resource **buffer, uint8_t **gpu, T_CONST *data,
                                     D3D12_CPU_DESCRIPTOR_HANDLE &descHandle = {})
    {
        const uint32_t constBufferSize = sizeof(T_CONST);

        ThrowIfFailed(
            device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                            &CD3DX12_RESOURCE_DESC::Buffer(constBufferSize),
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(buffer)));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvBufferDesc = {};
        cbvBufferDesc.BufferLocation                  = (*buffer)->GetGPUVirtualAddress();
        cbvBufferDesc.SizeInBytes                     = constBufferSize;
        device->CreateConstantBufferView(&cbvBufferDesc, descHandle);

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed((*buffer)->Map(0, &readRange, reinterpret_cast<void **>(gpu)));
        memcpy(*gpu, data, constBufferSize);
    }

    template <typename T>
    static void CreateDefaultBuffer(ID3D12Device *device, ID3D12Resource **buffer, T *data, uint32_t size)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(buffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8 *pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed((*buffer)->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, (void *)data, size);
        (*buffer)->Unmap(0, nullptr);
    }

    static ID3D12Resource *CreateTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                                         const std::string &filename, ID3D12Resource **texture,
                                         D3D12_CPU_DESCRIPTOR_HANDLE &descHandle, XMFLOAT3 color = {},
                                         bool isSRGB = false);
    static void CreateDDSTexture(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, std::wstring filename,
                                 ID3D12Resource **res, DescriptorHandle &handle);
    static void CreateDscriptor(ID3D12Device *device, uint32_t numDesc, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                D3D12_DESCRIPTOR_HEAP_FLAGS flag, ID3D12DescriptorHeap **descHeap);
    static void CreateShader(const std::wstring filename, ID3DBlob **vsShader, const std::string mainEntry,
                             const std::string version, std::vector<D3D_SHADER_MACRO> macro = {});
    static D3D12_VIEWPORT CreateViewport(const float xLT, const float yLT, const float w, const float h,
                                         const float minDepth = 0.0f, const float maxDepth = 1.0f);
    static D3D12_RECT CreateScissorRect(const long left, const long top, const long right, const long bottom);

  public:
    static uint32_t CheckMultiSample(ID3D12Device *device, DXGI_FORMAT format, uint32_t sampleCount);
};