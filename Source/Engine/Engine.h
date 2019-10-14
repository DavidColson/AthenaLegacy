#pragma once

#include "GameFramework/World.h"

#include <SDL.h>

struct IGame;

namespace Engine
{
  void SetCurrentScene(Scene* pScene);
	void Startup(IGame* pGame);
	void Run();
	void Shutdown();
}