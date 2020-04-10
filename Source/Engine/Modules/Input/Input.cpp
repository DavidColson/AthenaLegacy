
#include "Input.h"
#include "Engine.h"
#include "GraphicsDevice.h"

#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

InputState *pInput;

void Input::CreateInputState()
{
	pInput = new InputState();
}

bool Input::GetKeyDown(int keyCode)
{
	return pInput->keyDowns[keyCode];
}

bool Input::GetKeyUp(int keyCode)
{
	return pInput->keyUps[keyCode];
}

bool Input::GetKeyHeld(int keyCode)
{
	return pInput->keyStates[keyCode];
}

void Input::OnFrame(Scene& scene, float deltaTime)
{
	eastl::bitset<NKEYS> prevKeyStates = pInput->keyStates;

	// Copy the SDL keystate into our own bitset
	const Uint8 *pSdlKeyState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		pInput->keyStates[i] = bool(pSdlKeyState[i]);
	}

	// exclusive or, if state changed, then keychanges bit will be 1
	eastl::bitset<NKEYS> keyChanges = pInput->keyStates ^ prevKeyStates;

	// and, if key is down and it changed this frame, key went down
	pInput->keyDowns = keyChanges & pInput->keyStates;

	// and not, if key is not down and it changed this frame, key went up
	pInput->keyUps = keyChanges & ~pInput->keyStates;
}