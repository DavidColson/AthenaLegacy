#pragma once

#include <bitset>

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