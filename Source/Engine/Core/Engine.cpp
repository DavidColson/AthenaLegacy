#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <comdef.h>
#include <vector>
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

#include "EASTL/vector.h"
#include "EASTL/fixed_vector.h"

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
		Log::Print(Log::EErr, "%s failed to load", filename);
		return nullptr;
	}

	size_t fileSize = SDL_RWsize(rw);

	char* buffer = new char[fileSize];
	SDL_RWread(rw, buffer, sizeof(char) * fileSize, 1);
	SDL_RWclose(rw);
	buffer[fileSize] = '\0';
	return buffer;
}

void ImGuiPreUpdate(Scene& scene, float deltaTime)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void ImGuiRender(Scene& scene, float deltaTime)
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

	scene.RegisterReactiveSystem<CDebugDrawingState>(Reaction::OnAdd, DebugDraw::OnDebugDrawStateAdded);
	scene.RegisterReactiveSystem<CDebugDrawingState>(Reaction::OnRemove, DebugDraw::OnDebugDrawStateRemoved);

	scene.RegisterReactiveSystem<CShapesSystemState>(Reaction::OnAdd, Shapes::OnShapesSystemStateAdded);
	scene.RegisterReactiveSystem<CShapesSystemState>(Reaction::OnRemove, Shapes::OnShapesSystemStateRemoved);

	scene.RegisterReactiveSystem<CFontSystemState>(Reaction::OnAdd, FontSystem::OnAddFontSystemState);
	scene.RegisterReactiveSystem<CFontSystemState>(Reaction::OnRemove, FontSystem::OnRemoveFontSystemState);

	scene.RegisterSystem(SystemPhase::PreUpdate, ImGuiPreUpdate);
	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, Editor::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, Shapes::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, ParticlesSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, FontSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, DebugDraw::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, PostProcessingSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, ImGuiRender);

	scene.NewEntity("Engine Singletons");
	scene.Assign<CDebugDrawingState>(ENGINE_SINGLETON);
	scene.Assign<CShapesSystemState>(ENGINE_SINGLETON);
	scene.Assign<CFontSystemState>(ENGINE_SINGLETON);
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


	eastl::fixed_vector<int, 100> vec;

	vec.push_back(4);
	vec.push_back(2);
	vec.push_back(7);

	for (int i = 0; i < 100000; i++)
	{
		vec.push_back(2);
	}

	Log::Print(Log::EMsg, "Engine starting up");
	Log::Print(Log::EMsg, "Window size W: %.1f H: %.1f", width, height);

	GfxDevice::Initialize(g_pWindow, width, height);
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

		pCurrentScene->SimulateScene((float)frameTime);

		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

		pCurrentScene->RenderScene((float)frameTime);

		GfxDevice::PresentBackBuffer();
		GfxDevice::ClearRenderState();
		GfxDevice::PrintQueuedDebugMessages();

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
	AudioDevice::Destroy();
	GfxDevice::Destroy();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
