#pragma once

#include "SDL.h"

struct IGame;
struct Scene;

namespace Engine
{
	void Initialize();
	void Run(IGame* pGame, Scene* pInitialScene);
	void SetActiveScene(Scene* pScene);
}