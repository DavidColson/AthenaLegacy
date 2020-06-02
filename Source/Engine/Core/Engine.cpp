#include "Engine.h"

#include <SDL.h>
#include <Imgui/imgui.h> 
#include <Imgui/examples/imgui_impl_sdl.h>

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
	scene.RegisterSystem(SystemPhase::Update, Editor::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, TransformHeirarchy);

	// @Improvement consider allowing engine config files to change the update order of systems
}

void Engine::SetActiveScene(Scene* pScene)
{
	pPendingSceneLoad = pScene;
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
		0
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

	// Game update loop
	double frameTime = 0.016f;
	double targetFrameTime = 0.016f;
	while (g_gameRunning)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		AssetDB::UpdateHotReloading();

		RenderSystem::PreUpdate(*pCurrentScene, (float)frameTime);

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
					GfxDevice::ResizeWindow((float)event.window.data1, (float)event.window.data2);
					RenderSystem::OnWindowResize(*pCurrentScene, (float)event.window.data1, (float)event.window.data2);
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

		// Simulate current game scene
		pCurrentScene->SimulateScene((float)frameTime);
		Profiler::ClearFrameData();

		// Render the frame
		RenderSystem::OnFrame(*pCurrentScene, (float)frameTime);

		// Deal with scene loading
		if (pPendingSceneLoad)
		{
			delete pCurrentScene;
			pCurrentScene = pPendingSceneLoad;
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

	// Shutdown everything
	RenderSystem::Destroy();
	AudioDevice::Destroy();
	GfxDevice::Destroy();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
