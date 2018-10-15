#ifndef INPUTAPI_
#define INPUTAPI_

#include <bitset>

class InputAPI
{
public:
	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	void Update(bool& shutdownEngine);

private:
#define NKEYS 512
	std::bitset<NKEYS> keyDowns;
	std::bitset<NKEYS> keyUps;
	std::bitset<NKEYS> keyStates;
};

extern InputAPI gInputAPI;
#endif