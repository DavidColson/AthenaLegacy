#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <comdef.h>
#include <vector>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>

#include "Scene.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "Profiler.h"
#include "IGame.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	IGame* g_pGame{ nullptr };
	Scene* pCurrentScene{ nullptr };
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

void Engine::Run(IGame* pGame, Scene *pScene)
{	
	// Startup flow
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

	pCurrentScene = pScene;

	Graphics::CreateContext(g_pWindow, width, height);
	Input::CreateInputState();

	g_pGame = pGame;
	pGame->OnStart(*pCurrentScene);

	Graphics::OnGameStart(*pCurrentScene);

	// Game update loop
	double frameTime = 0.016f;
	double targetFrameTime = 0.016f;
	bool shutdown = false;
	while (!shutdown)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		// Update flow for the engine
		Graphics::OnFrameStart();
		Input::OnFrame(shutdown);
		g_pGame->OnFrame(*pCurrentScene, (float)frameTime);
		Editor::OnFrame(*pCurrentScene, shutdown, g_realFrameTime, g_observedFrameTime);
		Profiler::ClearFrameData();
		Graphics::OnFrame(*pCurrentScene, (float)frameTime); // This should take in the scene as a paramter and work from that, instead of

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

	// Shutdown everything
	g_pGame->OnEnd(*pCurrentScene);
	Graphics::OnGameEnd();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
