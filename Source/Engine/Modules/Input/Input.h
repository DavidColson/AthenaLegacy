#pragma once

#include <EASTL/bitset.h>
#include <Vec2.h>

struct Scene;

struct InputState
{
#define NKEYS 512
	eastl::bitset<NKEYS> keyDowns;
	eastl::bitset<NKEYS> keyUps;
	eastl::bitset<NKEYS> keyStates;

	float mouseXPos{ 0.0f };
	float mouseYPos{ 0.0f };
	float mouseXDelta{ 0.0f };
	float mouseYDelta{ 0.0f };
};

namespace Input
{
	void CreateInputState();

	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	Vec2f GetMouseDelta();
	bool GetMouseInRelativeMode();

	void OnFrame(Scene& scene, float deltaTime);
};