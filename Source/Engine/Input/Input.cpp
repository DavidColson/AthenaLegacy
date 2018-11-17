
#include "Input.h"

#include <windows.h>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>

#include "SDL.h"

InputState* pInput;

void Input::CreateInputState()
{
	pInput = new InputState();
}

bool Input::GetKeyDown(int keyCode)
{
	return pInput->m_keyDowns[keyCode];
}

bool Input::GetKeyUp(int keyCode)
{
	return pInput->m_keyUps[keyCode];
}

bool Input::GetKeyHeld(int keyCode)
{
	return pInput->m_keyStates[keyCode];
}

void Input::Update(bool& shutdownEngine)
{
	std::bitset<NKEYS> prevKeyStates = pInput->m_keyStates;
	
	// Copy the SDL keystate into our own bitset
	const Uint8* pSdlKeyState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		pInput->m_keyStates[i] = bool(pSdlKeyState[i]);
	}

	// exclusive or, if state changed, then keychanges bit will be 1
	std::bitset<NKEYS> keyChanges = pInput->m_keyStates ^ prevKeyStates;

	// and, if key is down and it changed this frame, key went down
	pInput->m_keyDowns = keyChanges & pInput->m_keyStates;

	// and not, if key is not down and it changed this frame, key went up
	pInput->m_keyUps = keyChanges & ~pInput->m_keyStates;

	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		/* an event was found */
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			shutdownEngine = true;
			break;
		}
	}
}