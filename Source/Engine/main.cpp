#include "SDL.h"
#include "SDL_syswm.h"

#include <comdef.h>
#include <vector>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>

#include "GameFramework/World.h"
#include "Input/Input.h"
#include "Maths/Maths.h"
#include "Renderer/Renderer.h"
#include "Editor/Editor.h"
#include "Log.h"

#include "Asteroids.h"

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

int main(int argc, char *argv[])
{
	// Engine Init
	// ***********

	SDL_Init(SDL_INIT_VIDEO);

	float width = 1800.0f;
	float height = 1000.0f;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		0
	);

	Log::Print(Log::EMsg, "Engine starting up");
	Log::Print(Log::EMsg, "Window size W: %.1f H: %.1f", width, height);

	Graphics::CreateContext(pWindow, width, height);
	Input::CreateInputState();

	Game::Startup();

	unsigned int start = SDL_GetTicks();

	float time = float(SDL_GetTicks() - start) / 1000.f;
	Log::Print(Log::EMsg, "Test Time Taken: %f", time);

	// Main Loop
	// *********

	float frameTime = 0.016f;
	float targetFrameTime = 0.016f;
	bool shutdown = false;
	while (!shutdown)
	{
		Graphics::NewFrame();

		unsigned int frameStart = SDL_GetTicks();
		Input::Update(shutdown);

		Game::Update(frameTime);

		Editor::ShowEditor(shutdown);

		Graphics::RenderFrame();

		float realframeTime = float(SDL_GetTicks() - frameStart) / 1000.f;
		//if (realframeTime < targetFrameTime)
		if (false)
		{
			frameTime = targetFrameTime;
			unsigned int waitTime = int((targetFrameTime - realframeTime) * 1000.f);
			SDL_Delay(waitTime);
		}
		else
		{
			frameTime = realframeTime;
		}
	}

	Graphics::Shutdown();

	SDL_DestroyWindow(pWindow);
	SDL_Quit();

	return 0;
}