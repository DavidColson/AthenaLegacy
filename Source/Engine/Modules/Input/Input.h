#pragma once

#include <EASTL/bitset.h>

struct Scene;

struct InputState
{
#define NKEYS 512
	eastl::bitset<NKEYS> keyDowns;
	eastl::bitset<NKEYS> keyUps;
	eastl::bitset<NKEYS> keyStates;
};

namespace Input
{
	void CreateInputState();

	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	void OnFrame(Scene& scene, float deltaTime);
};