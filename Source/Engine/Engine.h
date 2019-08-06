#pragma once

#include "SDL.h"

struct IGame;

namespace Engine
{
	void Startup(IGame* pGame);
	void Run();
	void Shutdown();
}