#pragma once

#include <bitset>

#include <SDL.h> // remove this and replace with custom keycodes

struct Scene;

struct InputState
{
#define NKEYS 512
	std::bitset<NKEYS> keyDowns;
	std::bitset<NKEYS> keyUps;
	std::bitset<NKEYS> keyStates;
};

namespace Input
{
	void CreateInputState();

	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	void OnFrame(Scene& scene, float deltaTime);
};