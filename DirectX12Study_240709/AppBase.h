#pragma once

#include "ColorBuffer.h"
#include "ConstantBuffer.h"
#include "DepthBuffer.h"
#include "DescriptorHeap.h"
#include "EventHandler.h"
#include "PostEffects.h"
#include "PostProcess.h"

extern AppBase* g_appBase;

class Model;
class Camera;
class Timer;
class ColorBuffer;
class FrameResource;

extern EventHandler g_EvnetHandler;

namespace Display
{
	extern uint32_t g_screenWidth;
	extern uint32_t g_screenHeight;
	extern float g_imguiWidth;
	extern float g_imguiHeight;
} // namespace Display

namespace Graphics
{
	extern ID3D12Device* g_Device;
	extern ColorBuffer g_DisplayPlane[];

	extern DescriptorHeap s_Texture;
	extern DescriptorHeap s_Sampler;
	extern DescriptorAllocator g_DescriptorAllocator[];
	inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDesciptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count = 1)
	{
		return g_DescriptorAllocator[type].Allocate(count);
	}
} // namespace Graphics

extern HWND g_hwnd;

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

	int32_t Run();

public:
	virtual bool Initialize();
	void Destroy();
	virtual void UpdateGui(const float frameRate);
	virtual void Render();
	virtual void Update(const float dt);

	LRESULT CALLBACK MemberWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void WaitForGpu();
	void InitCubemap(std::wstring basePath, std::wstring envFilename, std::wstring diffuseFilename,
		std::wstring specularFilename, std::wstring brdfFilename);
	virtual void InitLights();
	virtual void UpdateLights();
	void UpdateCamera(const float dt);
	void SetFrameResource(uint32_t numModels, uint32_t numLights);

private:
	bool InitWindow();
	bool InitD3D();
	void InitContext();
	virtual bool InitGui();

	void Resize();
	void CreateBuffers();
	void InitGlobalConsts();
	void InitSRVAandSamplerDesriptorHeap();

	void UpdateGlobalConsts(const float dt);

	void OnMouse(const float x, const float y);

	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

	void DestroyPSO();

protected:
	void BeginFrame();
	void MidFrame();
	void EndFrame();

protected:
	void RenderPostEffects(ID3D12GraphicsCommandList* cmdList);
	void RenderPostProcess(ID3D12GraphicsCommandList* cmdList);

public:
	static float GetAspect()
	{
		return ((float)Display::g_screenWidth - Display::g_imguiWidth) / Display::g_screenHeight;
	}
	static AppBase* Get() { return g_appBase; }
	void WorkerThread(int threadIndex);

protected:
	Camera* m_camera = nullptr;
	static const uint32_t s_frameCount = 2;
	// Pipeline objects.
	IDXGISwapChain1* m_swapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;

	GlobalConsts m_globalConstsData = {};
	GlobalConsts m_shadowConstsData[MAX_LIGHTS] = {};
	//UploadBuffer<GlobalConsts> m_globalConstsBuffer;
	//UploadBuffer<GlobalConsts> m_shadowConstBuffers;

	ColorBuffer m_floatBuffer;
	ColorBuffer m_resolvedBuffer;
	ColorBuffer m_postEffectsBuffer;
	DepthBuffer m_depthBuffer;
	DepthBuffer m_depthOnlyBuffer;
	DepthBuffer m_shadowMap[MAX_LIGHTS];
	DescriptorHeap m_imguiInitHeap;
	DescriptorHandle m_cubeMapHandle[4];
	ID3D12Resource* m_cubeMapResource[4] = {};
	int m_cubeMapType = 2;

	HWND m_hwnd = nullptr;
	bool m_useWarpDevice = false;
	// Gui control
	bool m_isFPV = false;
	bool m_drawAsNormal = false;
	bool m_isWireFrame = false;
	bool m_useMSAA = false;
	bool m_useTexture = true;
	// Mouse control
	bool m_leftButtonDown = false;
	bool m_rightButtonDown = false;
	bool m_leftButtonDragStart = false;
	bool m_rightButtonDragStart = false;
	float m_mouseX = 0.0f;
	float m_mouseY = 0.0f;
	float m_ndcX = 0.0f;
	float m_ndcY = 0.0f;

private:
	Timer* m_timer = nullptr;
	// Key control
	bool m_isKeyDown[256] = {};
	struct ThreadParameter
	{
		int threadIndex;
	};
	ThreadParameter m_threadParameters[g_NumContext];

protected:
	// Object list.
	Light m_light[3] = {};
	std::vector<Model*> m_lightSpheres = {};
	std::vector<Model*> m_opaqueList = {};
	Model* m_skybox = nullptr;
	Model* m_depthMap = nullptr;

	PostEffects m_postEffects;
	PostProcess m_postProcess;
	float m_gammaFactor = 2.2f;
	float m_exposureFactor = 1.0f;

	float m_metalness = 0.0f;
	float m_roughness = 0.0f;
	bool m_useNormalMap = false;

	std::vector<FrameResource*> m_frameResources = {};
	FrameResource* m_curFrameResource = nullptr;
	uint32_t m_curFrameResourceIndex = 0;

	// Synchronization objects.
	uint32_t m_frameIndex = 0;
	ID3D12Fence* m_fence = nullptr;
	uint64_t m_curFence = 0;

	HANDLE m_workerBeginRenderFrame[g_NumContext];
	HANDLE m_workerFinishShadowPass[g_NumContext];
	HANDLE m_workerFinishedRenderFrame[g_NumContext];
	HANDLE m_threadHandles[g_NumContext];
};
