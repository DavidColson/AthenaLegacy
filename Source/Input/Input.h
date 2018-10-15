#pragma once

#include <bitset>

class Input
{
public:
	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	void Update(bool& shutdownEngine);

private:
#define NKEYS 512
	std::bitset<NKEYS> m_keyDowns;
	std::bitset<NKEYS> m_keyUps;
	std::bitset<NKEYS> m_keyStates;
};

extern Input g_Input;