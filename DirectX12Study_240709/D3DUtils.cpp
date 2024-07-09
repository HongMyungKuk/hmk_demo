#include "pch.h"
#include "D3DUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void D3DUtils::CreateTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                             ID3D12CommandQueue *commandQueue, const std::string &filename, ID3D12Resource **texture,
                             D3D12_CPU_DESCRIPTOR_HANDLE &descHandle)
{
    int32_t width, height, channels;

    uint8_t *image = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    channels = 4;

    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels           = 1;
    textureDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width               = UINT64(width);
    textureDesc.Height              = UINT64(height);
    textureDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize    = 1;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                  D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr, IID_PPV_ARGS(texture)));

    // const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*texture, 0, 1);

    // Create the GPU upload buffer.Z
    ID3D12Resource *textureUploadHeap = nullptr;
    ThrowIfFailed(
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                        &CD3DX12_RESOURCE_DESC::Buffer(width * height * channels),
                                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData                  = (void *)image;
    textureData.RowPitch               = width * channels;
    textureData.SlicePitch             = textureData.RowPitch * height;

    UpdateSubresources(commandList, *texture, textureUploadHeap, 0, 0, 1, &textureData);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*texture, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format                          = textureDesc.Format;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels             = 1;
    device->CreateShaderResourceView(*texture, &srvDesc, descHandle);

    ThrowIfFailed(commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {commandList};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}
