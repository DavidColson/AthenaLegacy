#pragma once

#include "SDL.h"

struct IGame;
struct Scene;

namespace Engine
{
	void Run(IGame* pGame, Scene* pInitialScene);
}