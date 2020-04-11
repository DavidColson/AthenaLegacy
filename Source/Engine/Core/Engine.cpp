#include "Engine.h"

#include <SDL.h>
#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "Rendering/ParticlesSystem.h"
#include "Rendering/FontSystem.h"
#include "Rendering/PostProcessingSystem.h"
#include "Rendering/DebugDraw.h"
#include "Rendering/ShapesSystem.h"
#include "AudioDevice.h"
#include "Scene.h"
#include "Input/Input.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "Profiler.h"
#include "Memory.h"
#include "Maths.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	bool g_gameRunning{ true };

	EntityID g_engineSingleton{ INVALID_ENTITY };

	Scene* pCurrentScene{ nullptr };
	Scene* pPendingSceneLoad{ nullptr };
	SDL_Window* g_pWindow{ nullptr };
}

char* readFile(const char* filename)
{
	SDL_RWops* rw = SDL_RWFromFile(filename, "r+");
	if (rw == nullptr)
	{
		Log::Warn("%s failed to load", filename);
		return nullptr;
	}

	size_t fileSize = SDL_RWsize(rw);

	char* buffer = new char[fileSize];
	SDL_RWread(rw, buffer, sizeof(char) * fileSize, 1);
	SDL_RWclose(rw);
	buffer[fileSize] = '\0';
	return buffer;
}

void ImGuiPreUpdate(Scene& /* scene */, float /* deltaTime */)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void ImGuiRender(Scene& /* scene */, float /* deltaTime */)
{
	GFX_SCOPED_EVENT("Drawing imgui");
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnAdd, ParticlesSystem::OnAddEmitter);
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnRemove, ParticlesSystem::OnRemoveEmitter);

	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnAdd, PostProcessingSystem::OnAddPostProcessing);
	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnRemove, PostProcessingSystem::OnRemovePostProcessing);

	scene.RegisterSystem(SystemPhase::PreUpdate, ImGuiPreUpdate);
	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, Editor::OnFrame);

	// Need more manual control over render passes. This won't do
	scene.RegisterSystem(SystemPhase::Render, Shapes::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, ParticlesSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, FontSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, DebugDraw::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, PostProcessingSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, ImGuiRender);
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
	Shapes::Initialize();
	DebugDraw::Initialize();
	FontSystem::Initialize();

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
					PostProcessingSystem::OnWindowResize(*pCurrentScene, (float)event.window.data1, (float)event.window.data2);
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


		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

		// Render current game scene
		pCurrentScene->RenderScene((float)frameTime);

		GfxDevice::PresentBackBuffer();
		GfxDevice::ClearRenderState();
		GfxDevice::PrintQueuedDebugMessages();

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
	DebugDraw::Destroy();
	Shapes::Destroy();
	FontSystem::Destroy();
	AudioDevice::Destroy();
	GfxDevice::Destroy();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
