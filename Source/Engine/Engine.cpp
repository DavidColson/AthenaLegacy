#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <comdef.h>
#include <vector>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>

#include "GameFramework/World.h"
#include "Input/InputSystem.h"
#include "Input/InputComponents.h"
#include "Renderer/Renderer.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "IGame.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	IGame* g_pGame{ nullptr };
	SDL_Window* g_pWindow{ nullptr };
	Scene* g_pCurrentScene{ nullptr };
	// Another scene for application singletons?
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

void Engine::Startup(IGame* pGame, Scene* pInitialScene)
{	
	SetCurrentScene(pInitialScene);

	SDL_Init(SDL_INIT_VIDEO);

	float width = 1800.0f;
	float height = 1000.0f;

	g_pWindow = SDL_CreateWindow(
		"DirectX",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		0
	);

	Log::Print(Log::EMsg, "Engine starting up");
	Log::Print(Log::EMsg, "Window size W: %.1f H: %.1f", width, height);

	// This will send out engine wide events, with graphics/input systems picking up on that and running their code
	Graphics::CreateContext(g_pWindow, width, height);

	g_pGame = pGame;
	pGame->OnStart(g_pCurrentScene);
}

void Engine::Run()
{
	double frameTime = 0.016f;
	double targetFrameTime = 0.016f;
	bool shutdown = false;
	while (!shutdown)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		// Frame startup system (part of renderer module)
		Graphics::NewFrame();

		// Input system
		Input::Update(g_pCurrentScene, shutdown);

		g_pGame->OnFrame(g_pCurrentScene, (float)frameTime);

		// Editor draw system, again stores it's data in a singleton
		Editor::ShowEditor(g_pCurrentScene, shutdown, g_realFrameTime, g_observedFrameTime);

		// For now graphics is one system, storing it's data in a singleton
		// In future it may be several systems with phases for rendering
		Graphics::RenderFrame();

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
}

void Engine::Shutdown()
{
	g_pGame->OnEnd(g_pCurrentScene);
	Graphics::Shutdown();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}

void Engine::SetCurrentScene(Scene* pScene)
{
	g_pCurrentScene = pScene;
	Editor::SetCurrentScene(pScene);

	// Assign required scene singletons
	pScene->AssignSingleton<CInputState>();
}