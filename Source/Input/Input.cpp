
#include "Input.h"

#include <windows.h>

#include "SDL.h"

Input g_Input;

bool Input::GetKeyDown(int keyCode)
{
	return m_keyDowns[keyCode];
}

bool Input::GetKeyUp(int keyCode)
{
	return m_keyUps[keyCode];
}

bool Input::GetKeyHeld(int keyCode)
{
	return m_keyStates[keyCode];
}

void Input::Update(bool& shutdownEngine)
{
	std::bitset<NKEYS> prevKeyStates = m_keyStates;
	
	// Copy the SDL keystate into our own bitset
	const Uint8* pSdlKeyState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		m_keyStates[i] = bool(pSdlKeyState[i]);
	}

	// exclusive or, if state changed, then keychanges bit will be 1
	std::bitset<NKEYS> keyChanges = m_keyStates ^ prevKeyStates;

	// and, if key is down and it changed this frame, key went down
	m_keyDowns = keyChanges & m_keyStates;

	// and not, if key is not down and it changed this frame, key went up
	m_keyUps = keyChanges & ~m_keyStates;

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