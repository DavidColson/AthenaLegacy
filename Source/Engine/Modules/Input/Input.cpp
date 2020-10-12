
#include "Input.h"
#include "Engine.h"
#include "GraphicsDevice.h"
#include "Log.h"

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

Vec2f Input::GetMouseDelta()
{
	return Vec2f(pInput->mouseXDelta, pInput->mouseYDelta);
}

bool Input::GetMouseInRelativeMode()
{
	if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		return true;
	else
		return false;
}

void Input::OnFrame(Scene& scene, float deltaTime)
{
	eastl::bitset<NKEYS> prevKeyStates = pInput->keyStates;

	if (!ImGui::GetIO().WantCaptureKeyboard)
	{
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

		// Purely an engine editor feature for locking and unlocking the mouse. Should be removed from release builds
		if (GetKeyHeld(SDL_SCANCODE_LSHIFT) && GetKeyDown(SDL_SCANCODE_TAB))
		{
			if (GetMouseInRelativeMode())
				SDL_SetRelativeMouseMode(SDL_FALSE);
			else
				SDL_SetRelativeMouseMode(SDL_TRUE);
		}
	}

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		int x = 0;
		int y = 0;
		uint32_t mouseButtonState = SDL_GetMouseState(&x, &y);
		pInput->mouseXPos = (float)x;
		pInput->mouseYPos = (float)y;
		
		SDL_GetRelativeMouseState(&x, &y);
		pInput->mouseXDelta = (float)x;
		pInput->mouseYDelta = (float)y;
	}
}