#include "Engine.h"

#include <SDL.h>
#include <Imgui/imgui.h> 
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "GraphicsDevice.h"
#include "Rendering/RenderSystem.h"
#include "AudioDevice.h"
#include "Scene.h"
#include "Input/Input.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "Profiler.h"
#include "Memory.h"
#include "Maths.h"
#include "Matrix.h"
#include "Quat.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	bool g_gameRunning{ true };

	void (*pSceneCallBack)(Scene& scene);

	Scene* pCurrentScene{ nullptr };
	Scene* pPendingSceneLoad{ nullptr };
	SDL_Window* g_pWindow{ nullptr };
}

void Engine::GetFrameRates(double& outReal, double& outLimited)
{
	outReal = g_realFrameTime;
	outLimited = g_observedFrameTime;
}

void Engine::StartShutdown()
{
	g_gameRunning = false;
}

void Engine::NewSceneCreated(Scene& scene)
{
	RenderSystem::OnSceneCreate(scene);

	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, TransformHeirarchy);

	// @Improvement consider allowing engine config files to change the update order of systems
}

void Engine::SetActiveScene(Scene* pScene)
{
	pPendingSceneLoad = pScene;
}

void Engine::SetSceneCreateCallback(void (*pCallBackFunc)(Scene&))
{
	pSceneCallBack = pCallBackFunc;
}

void Engine::Initialize()
{
	// Startup flow
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	float width = 1800.0f;
	float height = 1000.0f;

	g_pWindow = SDL_CreateWindow(
		"Asteroids",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		SDL_WINDOW_RESIZABLE
	);

	Log::SetLogLevel(Log::EDebug);

	Log::Info("Engine starting up");
	Log::Info("Window size W: %.1f H: %.1f", width, height);

	GfxDevice::Initialize(g_pWindow, width, height);
	RenderSystem::Initialize();
	AudioDevice::Initialize();
	Input::CreateInputState();	
}

void Engine::Run(Scene *pScene)
{	
	pCurrentScene = pScene;

	if (pSceneCallBack)
		pSceneCallBack(*pCurrentScene);

	// Game update loop
	double frameTime = 0.016f;
	double targetFrameTime = 0.0166f;
	while (g_gameRunning)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		AssetDB::UpdateHotReloading();

		// Deal with events
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					if (event.window.windowID == SDL_GetWindowID(g_pWindow))
					{
						GfxDevice::ResizeWindow((float)event.window.data1, (float)event.window.data2);
					}
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				Engine::StartShutdown();
				break;
			}
		}

		// Note that pre-update has to happen after input processing or else imgui gets confused
		RenderSystem::PreUpdate(*pCurrentScene, (float)frameTime);

		// Simulate current game scene
		pCurrentScene->SimulateScene((float)frameTime);
		Profiler::ClearFrameData();

		// Render the frame
		RenderSystem::OnFrame(*pCurrentScene, (float)frameTime);

		if (Editor::IsInEditor())
		{
			Editor::FreeGameFrame();
			Editor::SetGameFrame(RenderSystem::GetGameFrame());
		}
		Editor::OnFrame(*pCurrentScene, (float)frameTime);

		// Render game frame onto screen
		{
			GfxDevice::SetBackBufferActive();
			GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });	
			GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

			// Consider imgui living "above" the render system. Such that when editor is active, we do the imgui frame drawing,
			// And the game render does it flow into a render target, and then editor renders after. Would require a lot of refactoring though... 
			{
				GFX_SCOPED_EVENT("Drawing imgui");
				ImGui::Render();
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}
			}

			GfxDevice::PresentBackBuffer();
			GfxDevice::ClearRenderState();
			GfxDevice::PrintQueuedDebugMessages();
		}

		// Deal with scene loading
		if (pPendingSceneLoad)
		{
			delete pCurrentScene;
			AssetDB::CollectGarbage();
			pCurrentScene = pPendingSceneLoad;
			if (pSceneCallBack)
				pSceneCallBack(*pCurrentScene);
			pPendingSceneLoad = nullptr;
		}

		// Framerate counter
		double realframeTime = double(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		if (realframeTime < targetFrameTime)
		{
			frameTime = targetFrameTime;
			unsigned int waitTime = int((targetFrameTime - realframeTime) * 1000.0);
			SDL_Delay(waitTime);
		}
		else
		{
			frameTime = realframeTime;
		}
		g_realFrameTime = realframeTime;
		g_observedFrameTime = double(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
	}

	delete pCurrentScene;
	AssetDB::CollectGarbage();

	// Shutdown everything
	RenderSystem::Destroy();
	AudioDevice::Destroy();
	GfxDevice::Destroy();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
