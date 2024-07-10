#include "pch.h"
#include "ModelViewer.h"
#include "GeometryGenerator.h"
#include "Model.h"

ModelViewer::ModelViewer() : AppBase()
{
}

ModelViewer::~ModelViewer()
{
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);

    SAFE_DELETE(m_model);
}

bool ModelViewer::Initialize()
{
    if (!AppBase::Initialize())
    {
        return false;
    }

    m_model = new Model;
    if (!m_model)
    {
        return false;
    }

    {
        MeshData cube = GeometryGenerator::MakeCube(1.0f, 1.0f, 1.0f);
        cube.albedoTextureFilename = "texture.jpg";
        m_model->Initialize(m_device, m_commandList, m_commandQueue, {cube});
        WaitForPreviousFrame();
    }

    return true;
}

void ModelViewer::Update()
{
    static float dt = 0.0f;
    dt += 1.0f / 60.0f;

    XMFLOAT3 eye = XMFLOAT3(0.0f, 0.0f, -3.0f);
    XMFLOAT3 dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 up  = XMFLOAT3(0.0f, 1.0f, 0.0f);

    auto eyeVec = XMLoadFloat3(&eye);
    auto dirVec = XMLoadFloat3(&dir);
    auto upVec  = XMLoadFloat3(&up);

    m_model->GetMeshConstCPU().world = XMMatrixTranspose(XMMatrixRotationY(dt));
    m_model->GetMeshConstCPU().view  = XMMatrixTranspose(XMMatrixLookToLH(eyeVec, dirVec, upVec));
    m_model->GetMeshConstCPU().projeciton =
        XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(70.0f), AppBase::GetAspect(), 0.01f, 1000.0f));

    m_model->Update();
}

void ModelViewer::Render()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_model->GetPSO()));

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_PRESENT,
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, false, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    // Record commands.
    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
                                         D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_model->Render(m_commandList);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex],
                                                                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                            D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();

    m_frameIndex = (m_frameIndex + 1) % s_frameCount;
}
