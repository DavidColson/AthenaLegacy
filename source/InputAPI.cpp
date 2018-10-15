
#include "InputAPI.h"

#include <windows.h>

#include "SDL.h"

InputAPI gInputAPI;

bool InputAPI::GetKeyDown(int keyCode)
{
	return keyDowns[keyCode];
}

bool InputAPI::GetKeyUp(int keyCode)
{
	return keyUps[keyCode];
}

bool InputAPI::GetKeyHeld(int keyCode)
{
	return keyStates[keyCode];
}

void InputAPI::Update(bool& shutdownEngine)
{
	std::bitset<NKEYS> prevKeyStates = keyStates;
	
	// Copy the SDL keystate into our own bitset
	const Uint8* sdlKeyState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		keyStates[i] = bool(sdlKeyState[i]);
	}

	// exclusive or, if state changed, then keychanges bit will be 1
	std::bitset<NKEYS> keyChanges = keyStates ^ prevKeyStates;

	// and, if key is down and it changed this frame, key went down
	keyDowns = keyChanges & keyStates;

	// and not, if key is not down and it changed this frame, key went up
	keyUps = keyChanges & ~keyStates;

	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		/* an event was found */
		switch (event.type) {
		case SDL_QUIT:
			shutdownEngine = true;
			break;
		}
	}
}