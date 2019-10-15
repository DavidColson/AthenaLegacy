#pragma once

#include <bitset>

#include <SDL.h> // remove this and replace with custom keycodes

struct InputState
{
#define NKEYS 512
	std::bitset<NKEYS> m_keyDowns;
	std::bitset<NKEYS> m_keyUps;
	std::bitset<NKEYS> m_keyStates;
};

namespace Input
{
	void CreateInputState();

	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	void Update(bool& shutdownEngine);
};