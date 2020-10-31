#pragma once

#include <EASTL/bitset.h>
#include <Vec2.h>

struct Scene;
union SDL_Event;

struct InputState
{
#define NKEYS 512
	eastl::bitset<NKEYS> keyDowns;
	eastl::bitset<NKEYS> keyUps;
	eastl::bitset<NKEYS> keyStates;

	eastl::bitset<5> mouseDowns;
	eastl::bitset<5> mouseUps;
	eastl::bitset<5> mouseStates;

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

	bool GetMouseDown(int buttonCode);
	bool GetMouseUp(int buttonCode);
	bool GetMouseHeld(int buttonCode);

	Vec2f GetMouseDelta();
	bool GetMouseInRelativeMode();

	void ClearState();
	void ClearHeldState();
	void ProcessEvent(SDL_Event* event);
};