#include "pch.h"

#include "AppBase.h"
#include "D3DUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <DirectXTexEXR.h>
#include <stb_image.h>

void ReadEXRImage(uint8_t **image, const std::string filename, int &w, int &h, int &c, DXGI_FORMAT &format);
void ReadImage(uint8_t **image, const std::string &filename, int &w, int &h, int &c);
void ReadImage(uint8_t **image, int &w, int &h, int &c, XMFLOAT3 color);
void ToLower(uint8_t **str);
size_t GetPixelSize(const DXGI_FORMAT pixelFormat);

ID3D12Resource *D3DUtils::CreateTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList,
                                        const std::string &filename, ID3D12Resource **texture,
                                        D3D12_CPU_DESCRIPTOR_HANDLE &descHandle, XMFLOAT3 color, bool isSRGB)

// 밉맵 적용되지 않음...
{
    int32_t width = 0, height = 0, channels = 0;

    uint8_t *image = nullptr;

    DXGI_FORMAT format = isSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    uint8_t *ext = Utils::get_extension(filename.c_str());
    ToLower(&ext);

    if (!filename.empty())
    {
        if (!strcmp((const char *)ext, "exr"))
            ReadEXRImage(&image, filename, width, height, channels, format);
        else
            ReadImage(&image, filename, width, height, channels);
    }
    else
    {
        ReadImage(&image, width, height, channels, color);
    }

    channels = 4;

    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels           = 1;
    textureDesc.Format              = format;
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

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*texture, 0, 1);

    // Create the GPU upload buffer.Z
    ID3D12Resource *textureUploadHeap = nullptr;
    ThrowIfFailed(
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData                  = (void *)image;
    textureData.RowPitch               = width * GetPixelSize(format);
    textureData.SlicePitch             = textureData.RowPitch * height;

    // 이후 컨텍스트로 관리
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

    SAFE_ARR_DELETE(image);

    return textureUploadHeap;
}

void D3DUtils::CreateDDSTexture(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, std::wstring filename,
                                ID3D12Resource **res, DescriptorHandle &handle)
{
    ResourceUploadBatch resourceUpload(device);

    resourceUpload.Begin();

    bool isCubemap = false;
    ThrowIfFailed(
        CreateDDSTextureFromFile(device, resourceUpload, filename.c_str(), res, false, 0, nullptr, &isCubemap));

    // Upload the resources to the GPU.
    auto uploadResourcesFinished = resourceUpload.End(cmdQueue);

    // Wait for the upload thread to terminate
    uploadResourcesFinished.wait();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension               = isCubemap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels       = (*res)->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format                          = (*res)->GetDesc().Format;

    handle = Graphics::s_Texture.Alloc(1);
    device->CreateShaderResourceView((*res), &srvDesc, D3D12_CPU_DESCRIPTOR_HANDLE(handle));
}

void D3DUtils::CreateDscriptor(ID3D12Device *device, uint32_t numDesc, D3D12_DESCRIPTOR_HEAP_TYPE type,
                               D3D12_DESCRIPTOR_HEAP_FLAGS flag, ID3D12DescriptorHeap **descHeap)
{
    D3D12_DESCRIPTOR_HEAP_DESC desciptorHeapDesc = {};
    desciptorHeapDesc.NumDescriptors             = numDesc;
    desciptorHeapDesc.Type                       = type;
    desciptorHeapDesc.Flags                      = flag;
    ThrowIfFailed(device->CreateDescriptorHeap(&desciptorHeapDesc, IID_PPV_ARGS(descHeap)));
}

void CheckResult(HRESULT hr, ID3DBlob *errorBlob)
{
    if (FAILED(hr))
    {
        // 파일이 없을 경우
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0)
        {
            std::cout << "File not found." << std::endl;
        }
    }
    // 에러 메시지가 있으면 출력
    if (errorBlob)
    {
        std::cout << "Shader compile error\n" << (char *)errorBlob->GetBufferPointer() << std::endl;
    }
}
void D3DUtils::CreateShader(const std::wstring filename, ID3DBlob **vsShader, const std::string mainEntry,
                            const std::string version, std::vector<D3D_SHADER_MACRO> macro)
{
#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ID3DBlob *errorBlob = nullptr;
    HRESULT hr =
        D3DCompileFromFile(filename.c_str(), macro.empty() ? nullptr : macro.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
                           mainEntry.c_str(), version.c_str(), compileFlags, 0, vsShader, &errorBlob);

    CheckResult(hr, errorBlob);

    // error 가 있는 상태에서 ptr 을 free하게 되면 에러 발생함.
    SAFE_DELETE(errorBlob);
}

D3D12_VIEWPORT D3DUtils::CreateViewport(const float xLT, const float yLT, const float w, const float h,
                                        const float minDepth, const float maxDepth)
{
    D3D12_VIEWPORT ret;
    ret.TopLeftX = xLT;
    ret.TopLeftY = yLT;
    ret.MinDepth = 0.0f;
    ret.MaxDepth = 1.0f;
    ret.Width    = (FLOAT)w;
    ret.Height   = (FLOAT)h;
    return ret;
}

D3D12_RECT D3DUtils::CreateScissorRect(const long left, const long top, const long right, const long bottom)
{
    D3D12_RECT ret;
    ret.left   = left;
    ret.top    = top;
    ret.right  = right;
    ret.bottom = bottom;

    return ret;
}

uint32_t D3DUtils::CheckMultiSample(ID3D12Device *device, DXGI_FORMAT format, uint32_t sampleCount)
{

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS info;
    info.Format           = format;
    info.SampleCount      = sampleCount;
    info.Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    info.NumQualityLevels = 0;

    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &info, sizeof(info)));

    return info.NumQualityLevels;
}

void ReadEXRImage(uint8_t **image, const std::string filename, int &w, int &h, int &c, DXGI_FORMAT &format)
{
    std::wstring wFilename = std::wstring(filename.begin(), filename.end());

    TexMetadata metaData;
    ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metaData));
    ScratchImage scratchImage;
    ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), &metaData, scratchImage));

    w      = int(metaData.width);
    h      = int(metaData.height);
    format = metaData.format;

    std::cout << filename << " " << w << " " << h << " " << format << std::endl;

    *image = new uint8_t[scratchImage.GetPixelsSize()];
    if (!(*image))
    {
        return;
    }
    memcpy(*image, scratchImage.GetPixels(), scratchImage.GetPixelsSize());
}

void ReadImage(uint8_t **image, const std::string &filename, int &w, int &h, int &c)
{
    uint8_t *img = stbi_load(filename.c_str(), &w, &h, &c, 0);

    if (img)
        std::cout << filename << " " << w << ", " << h << ", " << c << std::endl;
    else
        std::cout << filename << "is not found." << std::endl;

    uint64_t size = uint64_t(w * h * 4);

    *image = new uint8_t[size];

    assert(*image);

    if (c == 3)
    {
        for (int32_t j = 0; j < h; j++)
        {
            for (int32_t i = 0; i < w; i++)
            {
                for (int32_t k = 0; k < c; k++)
                {
                    (*image)[4 * (w * j + i) + k] = img[3 * (w * j + i) + k];
                }
                (*image)[4 * (w * j + i) + 3] = 255;
            }
        }
    }
    else if (c == 4)
    {
        for (int32_t j = 0; j < h; j++)
        {
            for (int32_t i = 0; i < w; i++)
            {
                for (int32_t k = 0; k < c; k++)
                {
                    (*image)[4 * (w * j + i) + k] = img[4 * (w * j + i) + k];
                }
            }
        }
    }
    else
    {
        std::cout << filename << " is not 3 or 4 channels. This image file`s channel is " << c << std::endl;
    }

    SAFE_ARR_DELETE(img);
}

void ReadImage(uint8_t **image, int &w, int &h, int &c, XMFLOAT3 color)
{
    w = 256;
    h = 256;
    c = 4;

    uint64_t size = uint64_t(w * h * c);

    *image = new uint8_t[size];
    if (!image)
    {
        return;
    }

    for (int32_t j = 0; j < h; j++)
    {
        for (int32_t i = 0; i < w; i++)
        {
            (*image)[4 * (w * j + i) + 0] = uint8_t(color.x * 255);
            (*image)[4 * (w * j + i) + 1] = uint8_t(color.y * 255);
            (*image)[4 * (w * j + i) + 2] = uint8_t(color.z * 255);
            (*image)[4 * (w * j + i) + 3] = 255;
        }
    }
}

void ToLower(uint8_t **str)
{
    size_t len = strlen((const char *)str);

    for (size_t i = 0; i < len; i++)
    {
        (*str)[i] = std::tolower((*str)[i]);
    }
}

size_t GetPixelSize(const DXGI_FORMAT pixelFormat)
{
    switch (pixelFormat)
    {
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return sizeof(uint16_t) * 4;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return sizeof(uint32_t) * 4;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return sizeof(uint8_t) * 4;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return sizeof(uint8_t) * 4;
    case DXGI_FORMAT_R32_SINT:
        return sizeof(int32_t) * 1;
    case DXGI_FORMAT_R16_FLOAT:
        return sizeof(uint16_t) * 1;
    }

    std::cout << "PixelFormat not implemented " << pixelFormat << std::endl;

    return sizeof(uint8_t) * 4;
}
