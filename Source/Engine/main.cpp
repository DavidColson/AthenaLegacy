#include "SDL.h"
#include "SDL_syswm.h"

#include <comdef.h>
#include <vector>

#include "GameFramework/World.h"
#include "Input/Input.h"
#include "Maths/Maths.h"
#include "Renderer/Renderer.h"

#include "Asteroids.h"

int main(int argc, char *argv[])
{
	// Engine Init
	// ***********

	SDL_Init(SDL_INIT_VIDEO);

	float width = 1000.0f;
	float height = 600.0f;

	SDL_Window *window = SDL_CreateWindow(
		"DirectX",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		0
	);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;


	Graphics::CreateContext(hwnd, width, height);
	Input::CreateInputState();

	Game::Startup();

	// Main Loop
	// *********


	float frameTime = 0.016f;
	float targetFrameTime = 0.016f;
	bool shutdown = false;
	while (!shutdown)
	{
		unsigned int frameStart = SDL_GetTicks();
		Input::Update(shutdown);

		Game::Update(frameTime);

		Graphics::RenderFrame();

		float realframeTime = float(SDL_GetTicks() - frameStart) / 1000.f;
		if (realframeTime < targetFrameTime)
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

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}