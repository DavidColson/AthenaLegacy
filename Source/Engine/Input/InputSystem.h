#pragma once

#include <bitset>
#include <SDL.h> // remove this and replace with custom keycodes

struct Scene;

namespace Input
{
  // Utilities provided by input module
	bool GetKeyDown(Scene* pScene, int keyCode);
	bool GetKeyUp(Scene* pScene, int keyCode);
	bool GetKeyHeld(Scene* pScene, int keyCode);

  // Actual input system
	void Update(Scene* pScene, bool& shutdownEngine);
};