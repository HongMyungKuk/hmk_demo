#include "pch.h"

#include "AppBase.h"
#include "Camera.h"
#include "ColorBuffer.h"
#include "DescriptorHeap.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "Input.h"
#include "Model.h"
#include "Timer.h"
#include "FrameResource.h"

AppBase* g_appBase = nullptr;
DXGI_FORMAT g_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
EventHandler g_EvnetHandler;

namespace Display
{
	uint32_t g_screenWidth = 1200;
	uint32_t g_screenHeight = 800;
	extern float g_imguiWidth = 0.0f;
	extern float g_imguiHeight = 0.0f;
} // namespace Display

namespace Graphics
{
	ID3D12Device* g_Device = nullptr;
	ColorBuffer g_DisplayPlane[2];

	DescriptorHeap s_Texture;
	DescriptorHeap s_Sampler;
	DescriptorAllocator g_DescriptorAllocator[] = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
												   D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
												   D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
} // namespace Graphics

HWND g_hwnd = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

AppBase::AppBase()
{
	g_appBase = this;
}

AppBase::~AppBase()
{
	for (int i = 0; i < 3; i++)
	{
		SAFE_DELETE(m_frameResources[i]);
	}

	Graphics::DestroyGraphicsCommon();

	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SAFE_VECTOR_CLEAR(m_lightSpheres);
	SAFE_VECTOR_CLEAR(m_opaqueList);
	SAFE_DELETE(m_depthMap);
	SAFE_DELETE(m_skybox);

	SAFE_DELETE(m_camera);
	for (uint32_t i = 0; i < 4; i++)
	{
		SAFE_RELEASE(m_cubeMapResource[i]);
	}
	SAFE_DELETE(m_timer);
	SAFE_RELEASE(m_fence);
	SAFE_RELEASE(m_commandList);
	SAFE_RELEASE(m_commandAllocator);
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_device);
}

bool AppBase::Initialize()
{
	if (!InitWindow())
	{
		return false;
	}
	if (!InitD3D())
	{
		return false;
	}
	if (!InitGui())
	{
		return false;
	}

	// Init Timer
	CREATE_OBJ(m_timer, Timer);
	m_timer->Initialize();

	// Mouse & Keyboard input initialize.
	GameInput::Initialize();

	this->InitGlobalConsts();

	// Init graphics common.
	Graphics::InitGraphicsCommon(m_device);

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));

	// Create sky box.
	CREATE_OBJ(m_skybox, Model);
	{
		MeshData cube = GeometryGenerator::MakeCube(100.0f, 100.0f, 100.0f);
		std::reverse(cube.indices.begin(), cube.indices.end());
		m_skybox->Initialize(m_device, m_commandList, { cube });
	}

	// Create depth map square
	CREATE_OBJ(m_depthMap, Model)
	{
		MeshData square = GeometryGenerator::MakeSquare(2.0f, 2.0f);
		m_depthMap->Initialize(m_device, m_commandList, { square }, {}, false, false);
	}

	// Initialize the post effects.
	m_postEffects.Initialize();
	// Initialize the post process.
	m_postProcess.Initialize(m_device, m_commandList, { &m_postEffectsBuffer },
		{ &Graphics::g_DisplayPlane[0] }, Display::g_screenWidth, Display::g_screenHeight, 4);

	return true;
}

void AppBase::Destroy()
{
	WaitForGpu();
}

void AppBase::Update(const float dt)
{
	m_curFrameResourceIndex = (m_curFrameResourceIndex + 1) % 3;
	m_curFrameResource = m_frameResources[m_curFrameResourceIndex];

	if (m_curFrameResource->m_fence != 0 && m_fence->GetCompletedValue() < m_curFrameResource->m_fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_curFrameResource->m_fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	m_timer->Update();

	GameInput::Update(dt);

	UpdateLights();

	UpdateGlobalConsts(dt);

	if (!m_opaqueList.empty())
	{
		m_opaqueList[0]->GetMaterialConstCPU().useNormalMap = m_useNormalMap;
		m_opaqueList[0]->GetMaterialConstCPU().useAlbedoMap = m_useTexture;
	}

	for (auto& e : m_opaqueList)
	{
		e->GetMaterialConstCPU().metalnessFactor = m_metalness;
		e->GetMaterialConstCPU().roughnessFactor = m_roughness;
		e->Update(m_curFrameResource->m_meshConstsBuffer, m_curFrameResource->m_materialConstsBuffer);
	}

	m_skybox->Update(m_curFrameResource->m_meshConstsBuffer, m_curFrameResource->m_materialConstsBuffer);
}

void AppBase::Render()
{
	BeginFrame();

#if SINGLETHREADED
	for (int i = 0; i < g_NumContext; i++)
	{
		WorkerThread(i);
	}

	MidFrame();
	EndFrame();
	m_commandQueue->ExecuteCommandLists(_countof(m_curFrameResource->m_batchSubmit), m_curFrameResource->m_batchSubmit);
#endif
	for (int i = 0; i < g_NumContext; i++)
	{
		SetEvent(m_workerBeginRenderFrame[i]); // Tell each worker to start drawing.
	}

	MidFrame();
	EndFrame();

	WaitForMultipleObjects(g_NumContext, m_workerFinishShadowPass, TRUE, INFINITE);

	m_commandQueue->ExecuteCommandLists(g_NumContext + 2, m_curFrameResource->m_batchSubmit); // Submit PRE, MID and shadows.

	WaitForMultipleObjects(g_NumContext, m_workerFinishedRenderFrame, TRUE, INFINITE);

	// Submit remaining command lists.
	m_commandQueue->ExecuteCommandLists(_countof(m_curFrameResource->m_batchSubmit) - g_NumContext - 2, m_curFrameResource->m_batchSubmit + g_NumContext + 2);
}

int32_t AppBase::Run()
{
	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Start the Dear ImGui frame
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGuiIO& io = ImGui::GetIO();
			(void)io;

			this->UpdateGui(io.Framerate);

			ImGui::Render();

			this->Update(ImGui::GetIO().DeltaTime);

			this->Render();

			ThrowIfFailed(m_commandAllocator->Reset());
			ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));

			ID3D12DescriptorHeap* descHeaps[] = { m_imguiInitHeap.Get() };
			m_commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList);

			// Indicate that the back buffer will now be used to present.
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				Graphics::g_DisplayPlane[m_frameIndex].GetResource(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			ThrowIfFailed(m_commandList->Close());
			// Execute the command list.
			ID3D12CommandList* ppCommandLists[] = { m_commandList };
			m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			// Present the frame.
			ThrowIfFailed(m_swapChain->Present(1, 0));

			m_frameIndex = (m_frameIndex + 1) % s_frameCount;

			m_curFrameResource->m_fence = ++m_curFence;

			m_commandQueue->Signal(m_fence, m_curFence);

			if (m_isKeyDown[VK_ESCAPE])
				break;
		}
	}
	this->Destroy();
	// Return this part of the WM_QUIT message to Windows.
	return 1;
}

bool AppBase::InitWindow()
{
	// Initialize the window class.
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DX12Study";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(Display::g_screenWidth), static_cast<LONG>(Display::g_screenHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	g_hwnd = m_hwnd =
		CreateWindow(windowClass.lpszClassName, windowClass.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
			CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
			nullptr, // We have no parent window.
			nullptr, // We aren't using menus.
			windowClass.hInstance, nullptr);
	ShowWindow(m_hwnd, SW_SHOW);

	return true;
}

bool AppBase::InitD3D()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			SAFE_RELEASE(debugController);
		}
	}
#endif
	IDXGIFactory6* factory = nullptr;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		IDXGIAdapter* warpAdapter = nullptr;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
		SAFE_RELEASE(warpAdapter);
	}
	else
	{
		IDXGIAdapter1* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
		SAFE_RELEASE(hardwareAdapter);
	}

	Graphics::g_Device = m_device;

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = s_frameCount;
	swapChainDesc.Width = Display::g_screenWidth;
	swapChainDesc.Height = Display::g_screenHeight;
	swapChainDesc.Format = g_BackBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain1* swapChain = nullptr;
	ThrowIfFailed(
		factory->CreateSwapChainForHwnd(m_commandQueue, m_hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
	m_swapChain = swapChain;
	swapChain = nullptr;

	ThrowIfFailed(
		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	m_frameIndex = 0;

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator,
		nullptr, IID_PPV_ARGS(&m_commandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_commandList->Close());

	// Create synchronization objects.
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	InitSRVAandSamplerDesriptorHeap();

	this->Resize();

	SAFE_RELEASE(factory);

	return true;
}

void AppBase::InitContext()
{
#if !SINGLETHREADED
	struct threadwrapper
	{
		static unsigned int WINAPI thunk(LPVOID lpParameter)
		{
			ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
			AppBase::Get()->WorkerThread(parameter->threadIndex);
			return 0;
		}
	};

	for (int i = 0; i < g_NumContext; i++)
	{
		m_workerBeginRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishedRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishShadowPass[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_threadParameters[i].threadIndex = i;

		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr,
			0,
			threadwrapper::thunk,
			reinterpret_cast<LPVOID>(&m_threadParameters[i]),
			0,
			nullptr));

		assert(m_workerBeginRenderFrame[i] != NULL);
		assert(m_workerFinishedRenderFrame[i] != NULL);
		assert(m_threadHandles[i] != NULL);

	}
#endif
}

bool AppBase::InitGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	m_imguiInitHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hwnd);
	ImGui_ImplDX12_Init(m_device, 3, DXGI_FORMAT_R8G8B8A8_UNORM, m_imguiInitHeap.Get(),
		D3D12_CPU_DESCRIPTOR_HANDLE(m_imguiInitHeap[0]),
		D3D12_GPU_DESCRIPTOR_HANDLE(m_imguiInitHeap[0]));
	return true;
}

void AppBase::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	IDXGIAdapter1* adapter = nullptr;

	IDXGIFactory6* factory6 = nullptr;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
			adapterIndex,
			requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
			: DXGI_GPU_PREFERENCE_UNSPECIFIED,
			IID_PPV_ARGS(&adapter)));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter;
	adapter = nullptr;

	SAFE_RELEASE(factory6);
}

void AppBase::InitGlobalConsts()
{
	//m_globalConstsBuffer.Initialize(m_device, 1);
	//m_shadowConstBuffers.Initialize(m_device, MAX_LIGHTS);
}

void AppBase::UpdateGlobalConsts(const float dt)
{
	auto eyePos = m_camera->GetPosition();
	auto viewRow = m_camera->GetViewMatrix();
	auto projRow = m_camera->GetProjectionMatrix();

	// global consts data.
	m_globalConstsData.eyeWorld = eyePos;
	m_globalConstsData.view = viewRow.Transpose();
	m_globalConstsData.viewInv = m_globalConstsData.view.Invert();
	m_globalConstsData.proj = projRow.Transpose();
	m_globalConstsData.projInv = m_globalConstsData.proj.Invert();
	m_globalConstsData.viewProjInv = (viewRow * projRow).Invert().Transpose();
	m_globalConstsData.envType = (uint8_t)m_cubeMapType;
	m_globalConstsData.dt += dt;

	// shadow consts data.
	m_shadowConstsData[0] = m_globalConstsData;

	Vector3 lightCenter(0.0f);
	if (!m_opaqueList.empty() && m_opaqueList[0] != nullptr)
		lightCenter = m_opaqueList[0]->GetPos();

	eyePos = m_light[1].position;
	viewRow = XMMatrixLookAtLH(eyePos, lightCenter, Vector3(0.0f, 0.0f, 1.0f)); // 물체의 중심을 향해
	projRow = XMMatrixPerspectiveFovLH(XMConvertToRadians(120.0f), 1.0f, 0.01f, 1000.0f);

	// m_shadowConstsData[1].eyeWorld = eyePos;
	m_shadowConstsData[1].view = viewRow.Transpose();
	m_shadowConstsData[1].viewInv = m_shadowConstsData[1].view.Invert();
	m_shadowConstsData[1].proj = projRow.Transpose();
	m_shadowConstsData[1].projInv = m_shadowConstsData[1].proj.Invert();

	eyePos = m_light[2].position;
	viewRow = XMMatrixLookAtLH(eyePos, Vector3(0.0f), Vector3(1.0f, 0.0f, 0.0f)); // 물체의 중심을 향해
	projRow = XMMatrixPerspectiveFovLH(XMConvertToRadians(120.0f), 1.0f, 0.01f, 1000.0f);

	m_shadowConstsData[2].view = viewRow.Transpose();
	m_shadowConstsData[2].viewInv = m_shadowConstsData[2].view.Invert();
	m_shadowConstsData[2].proj = projRow.Transpose();
	m_shadowConstsData[2].projInv = m_shadowConstsData[2].proj.Invert();

	// shadow matrix.
	m_globalConstsData.lights[1].view = m_shadowConstsData[1].view;
	m_globalConstsData.lights[1].proj = m_shadowConstsData[1].proj;
	m_globalConstsData.lights[2].view = m_shadowConstsData[2].view;
	m_globalConstsData.lights[2].proj = m_shadowConstsData[2].proj;

	// update to gpu.
	m_curFrameResource->m_globalConstsBuffer->Upload(0, &m_globalConstsData);
	for (uint32_t i = 0; i < 3; i++)
	{
		m_curFrameResource->m_shadowConstsBuffer->Upload(i, &m_shadowConstsData[i]);
	}
}

void AppBase::UpdateCamera(const float dt)
{
	if (GameInput::IsPressed(GameInput::kKey_w))
	{
		m_camera->MoveFront(dt);
	}
	if (GameInput::IsPressed(GameInput::kKey_s))
	{
		m_camera->MoveBack(dt);
	}
	if (GameInput::IsPressed(GameInput::kKey_d))
	{
		m_camera->MoveRight(dt);
	}
	if (GameInput::IsPressed(GameInput::kKey_a))
	{
		m_camera->MoveLeft(dt);
	}
	if (GameInput::IsPressed(GameInput::kKey_q))
	{
		m_camera->MoveUp(dt);
	}
	if (GameInput::IsPressed(GameInput::kKey_e))
	{
		m_camera->MoveDown(dt);
	}
}

void AppBase::SetFrameResource(uint32_t numModels, uint32_t numLights)
{
	for (uint32_t i = 0; i < 3; i++)
	{
		FrameResource* newFrameResource = new FrameResource(int(numModels), int(numLights), Graphics::defaultSolidPSO, Graphics::depthOnlyPSO);
		m_frameResources.push_back(newFrameResource);
	}
}

void AppBase::UpdateGui(const float frameRate)
{
	using namespace Display;

	ImGuiWindowFlags window_flags = 0;
	if (false)
		window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (false)
		window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (true)
		window_flags |= ImGuiWindowFlags_MenuBar;
	if (false)
		window_flags |= ImGuiWindowFlags_NoMove;
	if (false)
		window_flags |= ImGuiWindowFlags_NoResize;
	if (false)
		window_flags |= ImGuiWindowFlags_NoCollapse;
	if (false)
		window_flags |= ImGuiWindowFlags_NoNav;
	if (false)
		window_flags |= ImGuiWindowFlags_NoBackground;
	if (false)
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	if (false)
		window_flags |= ImGuiWindowFlags_UnsavedDocument;

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Gui Demo", nullptr, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		// ImGui::End(); // => many end warning.
		return;
	}

	Graphics::mainViewport =
		D3DUtils::CreateViewport(0.0f, 0.0f, (float)(g_screenWidth - g_imguiWidth), (float)g_screenHeight);
	Graphics::mainSissorRect =
		D3DUtils::CreateScissorRect(0, 0, (long)(g_screenWidth - g_imguiWidth), (long)g_screenHeight);

	ImGui::Text("App average %.3f ms/frame (%.1f FPS)", 1000.0f / frameRate, frameRate);
	auto cameraSpeed = m_camera->GetCameraSpeed();
	// ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.0001f, 0.001f, "%.4f");
	ImGui::SliderFloat("Camera speed", &cameraSpeed, 1.0f, 3.0f, "%.4f");
	if (cameraSpeed != m_camera->GetCameraSpeed())
	{
		m_camera->SetCameraSpeed(cameraSpeed);
	}

	if (ImGui::RadioButton("env", m_cubeMapType == 0))
	{
		m_cubeMapType = 0;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("specular", m_cubeMapType == 1))
	{
		m_cubeMapType = 1;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("diffuse", m_cubeMapType == 2))
	{
		m_cubeMapType = 2;
	}

	if (ImGui::CollapsingHeader("Image Processiong"))
	{
		ImGui::SliderFloat("Exposure", &m_exposureFactor, 0.0f, 10.0f);
		ImGui::SliderFloat("Gamma collection", &m_gammaFactor, 0.01f, 5.0f);
		//float blurStrength = 0.0f;
		ImGui::SliderFloat("Blur strength", &m_postProcess.m_combineFilter.m_constData.strength, 0.0f, 1.0f);
		//if (m_postProcess.m_combineFilter.m_constData.strength != blurStrength)
		{
			m_postProcess.m_combineFilter.m_constsBuffer.Upload(0, &m_postProcess.m_combineFilter.m_constData);
		}
	}

	if (ImGui::CollapsingHeader("Post Effects"))
	{
		ImGui::SliderFloat("Fog Strength", &m_globalConstsData.fogStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Halo Radius", &m_globalConstsData.haloRadius, 0.0f, 50.0f);
		ImGui::SliderFloat("Halo Strength", &m_globalConstsData.haloStrength, 0.0f, 5.0f);
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::Checkbox("Normal map", &m_useNormalMap);
		if (!m_opaqueList.empty())
		{
			ImGui::Checkbox("Height map", (bool*)&m_opaqueList[0]->GetMeshConstCPU().useHeightMap);
			ImGui::SliderFloat("Height scale", &m_opaqueList[0]->GetMeshConstCPU().heightScale, 0.0f, 1.0f);
		}
		ImGui::SliderFloat("MipMap", &m_globalConstsData.mipmap, 0.0f, 5.0f);
		ImGui::SliderFloat("Env strength", &m_globalConstsData.envStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Metalness", &m_metalness, 0.0f, 1.0f);
		ImGui::SliderFloat("Roughness", &m_roughness, 0.0f, 1.0f);
	}
}

void AppBase::RenderPostEffects(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->RSSetViewports(1, &Graphics::mainViewport);
	cmdList->RSSetScissorRects(1, &Graphics::mainSissorRect);
	// PSO 설정
	cmdList->SetPipelineState(Graphics::postEffectsPSO);

	// Transition the resource from its initial state to be used as a depth buffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap[0].GetResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_floatBuffer.GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_resolvedBuffer.GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST));
	// float msaa on => float mass off
	cmdList->ResolveSubresource(m_resolvedBuffer.GetResource(), 0,
		m_floatBuffer.GetResource(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_resolvedBuffer.GetResource(),
		D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_floatBuffer.GetResource(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET));
	// post effects rtv 에 출력
	cmdList->SetGraphicsRootDescriptorTable(3, m_resolvedBuffer.GetSRV());
	cmdList->SetGraphicsRootDescriptorTable(5, D3D12_GPU_DESCRIPTOR_HANDLE(m_shadowMap[0].GetSRV()));
	cmdList->OMSetRenderTargets(1, &m_postEffectsBuffer.GetRTV(), false, nullptr);

	m_postEffects.Render(cmdList);

	// Transition the resource from its initial state to be used as a depth buffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap[0].GetResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void AppBase::RenderPostProcess(ID3D12GraphicsCommandList* cmdList)
{
	m_postProcess.Render(cmdList, m_frameIndex);
}

void AppBase::DestroyPSO()
{
	SAFE_RELEASE(Graphics::defaultWirePSO);
	SAFE_RELEASE(Graphics::defaultSolidPSO);
}

void AppBase::BeginFrame()
{
	m_curFrameResource->Init();

	ThrowIfFailed(m_curFrameResource->m_commandLists[CommandListPre]->Close());
}

void AppBase::MidFrame()
{
	ThrowIfFailed(m_curFrameResource->m_commandLists[CommandListMid]->Close());
}

void AppBase::EndFrame()
{
	ThrowIfFailed(m_curFrameResource->m_commandLists[CommandListPost]->Close());
}

void AppBase::CreateBuffers()
{
	using namespace Display;
	using namespace Graphics;

	// Initialize Shader resource view and sampler descriptor heap.
	// this->InitSRVAandSamplerDesriptorHeap();

	// Create a RTV for each frame.
	D3D12_RESOURCE_DESC rtvDesc = {};

	// refactorying
	uint32_t numQualityLevels = D3DUtils::CheckMultiSample(m_device, DXGI_FORMAT_R16G16B16A16_FLOAT, 4);

	m_useMSAA = (numQualityLevels > 0) ? true : false;

	// m_useMSAA = false;

	for (UINT n = 0; n < s_frameCount; n++)
	{
		ID3D12Resource* backBuffer = nullptr;
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)));
		g_DisplayPlane[n].CreateFromSwapChain(backBuffer);
		rtvDesc = backBuffer->GetDesc();
	}

	m_floatBuffer.Create(rtvDesc, m_useMSAA); // Multi sampling on.
	m_resolvedBuffer.Create(rtvDesc, false);  // Multi sampling off.
	m_postEffectsBuffer.Create(rtvDesc, false);

	// Create depth stencil buffer.
	m_depthBuffer.Create(g_screenWidth, g_screenHeight, DXGI_FORMAT_R24G8_TYPELESS, false, m_useMSAA);
	for (uint32_t i = 0; i < MAX_LIGHTS; i++)
	{
		// 0    : depth only buffer
		// 1 ~  : shadow map
		m_shadowMap[i].Create(1024, 1024, DXGI_FORMAT_R32_TYPELESS, true, false);
	}
}

void AppBase::OnMouse(const float x, const float y)
{
	using namespace Display;

	auto newScreenWidth = float(g_screenWidth - g_imguiWidth);
	auto newSreenHeight = float(g_screenHeight);

	m_mouseX = std::clamp(x, 0.0f, newScreenWidth);
	m_mouseY = std::clamp(y, 0.0f, newSreenHeight);

	m_ndcX = m_mouseX / newScreenWidth * 2.0f - 1.0f;
	m_ndcY = -(m_mouseY / newSreenHeight * 2.0f - 1.0f);

	m_ndcX = std::clamp(m_ndcX, -1.0f, 1.0f);
	m_ndcY = std::clamp(m_ndcY, -1.0f, 1.0f);

	if (m_isFPV)
	{
		m_camera->MouseUpdate(m_ndcX, m_ndcY);
	}
}

void AppBase::InitSRVAandSamplerDesriptorHeap()
{
	Graphics::s_Texture.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4098 * 2);
	Graphics::s_Sampler.Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 256);

	// Create sampler
	DescriptorHandle samplerHandle = Graphics::s_Sampler.Alloc(1);

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.BorderColor[0] = 100.0f; // 큰 Z값 => 현재 동작하지 않음.
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

	Graphics::g_Device->CreateSampler(&samplerDesc, samplerHandle);
}

LRESULT AppBase::MemberWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_SIZE: {
		//using namespace Display;

		//RECT rect = {};
		//GetClientRect(hWnd, &rect);
		//g_screenWidth = uint32_t(rect.right - rect.left);
		//g_screenHeight = uint32_t(rect.bottom - rect.top);

		//if (m_swapChain)
		//{
		//	if (g_screenWidth && g_screenHeight)
		//	{
		//		this->Resize();

		//		Graphics::mainViewport =
		//			D3DUtils::CreateViewport(0.0f, 0.0f, (float)g_screenWidth, (float)g_screenHeight);
		//		Graphics::mainSissorRect = D3DUtils::CreateScissorRect(0, 0, (long)g_screenWidth, (long)g_screenHeight);
		//	}
		//}
	}
				break;
	case WM_KEYDOWN:
		if (wParam == 70)
			m_isFPV = !m_isFPV;
		m_isKeyDown[wParam] = true;
		break;
	case WM_KEYUP:
		m_isKeyDown[wParam] = false;
		break;
	case WM_MOUSEMOVE:
		// std::cout << HIWORD(lParam) << " " << LOWORD(lParam) << std::endl;
		OnMouse(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		m_leftButtonDown = true;
		m_leftButtonDragStart = true;
		break;
	case WM_LBUTTONUP:
		m_leftButtonDown = false;
		break;
	case WM_RBUTTONDOWN:
		m_rightButtonDown = true;
		m_rightButtonDragStart = true;
		break;
	case WM_RBUTTONUP:
		m_rightButtonDown = false;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void AppBase::WaitForGpu()
{
	m_curFence++;

	ThrowIfFailed(m_commandQueue->Signal(m_fence, m_curFence));

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < m_curFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		assert(eventHandle);

		ThrowIfFailed(m_fence->SetEventOnCompletion(m_curFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void AppBase::InitCubemap(std::wstring basePath, std::wstring envFilename, std::wstring diffuseFilename,
	std::wstring specularFilename, std::wstring brdfFilename)
{
	D3DUtils::CreateDDSTexture(m_device, m_commandQueue, (basePath + envFilename).c_str(), &m_cubeMapResource[0],
		m_cubeMapHandle[0]);
	D3DUtils::CreateDDSTexture(m_device, m_commandQueue, (basePath + diffuseFilename).c_str(), &m_cubeMapResource[1],
		m_cubeMapHandle[1]);
	D3DUtils::CreateDDSTexture(m_device, m_commandQueue, (basePath + specularFilename).c_str(), &m_cubeMapResource[2],
		m_cubeMapHandle[2]);
	D3DUtils::CreateDDSTexture(m_device, m_commandQueue, (basePath + brdfFilename).c_str(), &m_cubeMapResource[3],
		m_cubeMapHandle[3]);
}

void AppBase::InitLights()
{
	// directional light
	{
		m_light[0].type |= DIRECTIONAL_LIGHT;
	}
	// point light
	{
		m_light[1].type |= POINT_LIGHT;
		m_light[1].type |= SHADOW_MAP;
		m_light[1].position = Vector3(0.0f, 2.5f, -1.0f);

		MeshData sphere = GeometryGenerator::MakeSphere(0.025f, 10, 10);
		Model* lightSphere = new Model;
		lightSphere->Initialize(m_device, m_commandList, { sphere });
		lightSphere->GetMaterialConstCPU().albedoFactor = Vector3(0.0f);
		lightSphere->GetMaterialConstCPU().emissionFactor = Vector3(1.0f, 1.0f, 0.0f);
		lightSphere->UpdateWorldMatrix(Matrix::CreateTranslation(m_light[1].position));
		lightSphere->m_castShadow = false;
		m_lightSpheres.push_back(lightSphere);
	}
	// spot light
	{
		m_light[2].type |= SPOT_LIGHT;
		m_light[2].type |= SHADOW_MAP;
		m_light[2].position = Vector3(0.0f, 3.0f, 0.0f);
		m_light[2].direction = Vector3(0.0f, -1.0f, 0.0f);

		MeshData sphere = GeometryGenerator::MakeSphere(0.025f, 10, 10);
		Model* lightSphere = new Model;
		lightSphere->Initialize(m_device, m_commandList, { sphere });
		lightSphere->GetMaterialConstCPU().albedoFactor = Vector3(0.0f);
		lightSphere->GetMaterialConstCPU().emissionFactor = Vector3(1.0f, 1.0f, 0.0f);
		lightSphere->UpdateWorldMatrix(Matrix::CreateTranslation(m_light[2].position));
		lightSphere->m_castShadow = false;
		m_lightSpheres.push_back(lightSphere);
	}
}

void AppBase::UpdateLights()
{
}

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_appBase->MemberWndProc(hWnd, message, wParam, lParam);
}

void AppBase::Resize()
{
	using namespace Display;
	using namespace Graphics;

	assert(g_Device);
	assert(m_swapChain);
	assert(m_commandAllocator);

	WaitForGpu();

	m_commandList->Reset(m_commandAllocator, nullptr);

	// Reset a RTV for each frame.
	if (g_DisplayPlane[0].GetResource())
	{
		for (UINT n = 0; n < s_frameCount; n++)
		{
			g_DisplayPlane[n].GetResource()->Release();
		}
	}
	if (m_depthBuffer.GetResource())
	{
		m_depthBuffer.GetResource()->Release();
	}
	// swap chain resize.
	if (m_swapChain)
	{
		ThrowIfFailed(m_swapChain->ResizeBuffers(s_frameCount, g_screenWidth, g_screenHeight, g_BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	}

	// Create frame resources.
	{
		this->CreateBuffers();

		// generic depth stencil buffer.
		// Transition the resource from its initial state to be used as a depth buffer.
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.GetResource(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));
		// shaddow map.
		// Transition the resource from its initial state to be used as a depth buffer.
		for (uint32_t i = 0; i < MAX_LIGHTS; i++)
		{
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap[i].GetResource(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE));
		}
	}

	// For texture loading.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForGpu();
}

void AppBase::WorkerThread(int threadIndex)
{
	WaitForSingleObject(m_workerBeginRenderFrame[threadIndex], INFINITE);

	ID3D12GraphicsCommandList* pShadowCommandList = m_curFrameResource->m_shadowCommandLists[threadIndex];
	ID3D12GraphicsCommandList* pSceneCommandList = m_curFrameResource->m_sceneCommandLists[threadIndex];


	pShadowCommandList->RSSetViewports(1, &Graphics::shadowViewport);
	pShadowCommandList->RSSetScissorRects(1, &Graphics::shadowSissorRect);
	pShadowCommandList->SetGraphicsRootSignature(Graphics::defaultRootSignature);
	pShadowCommandList->SetGraphicsRootDescriptorTable(3, m_cubeMapHandle[0]);

	for (uint32_t i = 0; i < MAX_LIGHTS; i++)
	{
		pShadowCommandList->SetGraphicsRootConstantBufferView(0, m_curFrameResource->m_shadowConstsBuffer->GetResource()->GetGPUVirtualAddress() +
			i * sizeof(GlobalConsts));
		pShadowCommandList->OMSetRenderTargets(0, nullptr, false, &m_shadowMap[i].GetDSV());
		pShadowCommandList->ClearDepthStencilView(m_shadowMap[i].GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// render object.
		for (auto& e : m_opaqueList)
		{
			if (e->m_castShadow)
			{
				pShadowCommandList->SetPipelineState(e->GetDepthOnlyPSO());
				e->Render(pShadowCommandList);
			}
		}
		// render skybox
		pShadowCommandList->SetPipelineState(Graphics::depthOnlyPSO);
		m_skybox->Render(pShadowCommandList);
	}

	ThrowIfFailed(pShadowCommandList->Close());

#if !SINGLETHREADED
	// Submit shadow pass.
	SetEvent(m_workerFinishShadowPass[threadIndex]);
#endif

	pSceneCommandList->RSSetViewports(1, &Graphics::mainViewport);
	pSceneCommandList->RSSetScissorRects(1, &Graphics::mainSissorRect);

	pSceneCommandList->OMSetRenderTargets(1, &m_floatBuffer.GetRTV(), false, &m_depthBuffer.GetDSV());
	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	pSceneCommandList->ClearRenderTargetView(m_floatBuffer.GetRTV(), clearColor, 0, nullptr);
	pSceneCommandList->ClearDepthStencilView(m_depthBuffer.GetDSV(), D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0, nullptr);

	pSceneCommandList->SetGraphicsRootConstantBufferView(0, m_curFrameResource->m_globalConstsBuffer->GetResource()->GetGPUVirtualAddress());
	// shadow map srv.
	pSceneCommandList->SetGraphicsRootDescriptorTable(5, D3D12_GPU_DESCRIPTOR_HANDLE(m_shadowMap[0].GetSRV()));
	pSceneCommandList->SetGraphicsRootDescriptorTable(6, D3D12_GPU_DESCRIPTOR_HANDLE(Graphics::s_Sampler[0]));

	// render object.
	int count = 0;
	for (auto& e : m_opaqueList)
	{
		if (e->m_isDraw == true)
		{
			pSceneCommandList->SetPipelineState(e->GetPSO(m_isWireFrame));
			e->Render(pSceneCommandList);

			if (m_drawAsNormal)
			{
				pSceneCommandList->SetPipelineState(Graphics::normalPSO);
				e->RenderNormal(pSceneCommandList);
			}
		}
		//else
		//{
		//	std::cout << "Number of frustum culling object. " << std::endl;
		//	count++;
		//	std::cout << count << std::endl;
		//}
	}
	// render skybox
	pSceneCommandList->SetPipelineState(Graphics::skyboxPSO);
	m_skybox->Render(pSceneCommandList);
}