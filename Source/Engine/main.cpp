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

	bool shutdown = false;
	while (!shutdown)
	{
		Input::Update(shutdown);

		Game::Update();

		Graphics::RenderFrame();
	}

	Graphics::Shutdown();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}