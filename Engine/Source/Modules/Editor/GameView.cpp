#include "GameView.h"

#include "Rendering/GameRenderer.h"
#include "GraphicsDevice.h"
#include "Engine.h"
#include "Input/Input.h"

#include <ImGui/imgui.h>
#include <SDL.h>

namespace
{
	Vec2f windowSizeCache{ Vec2f(100.0f, 100.0f) };
	Vec2f gameSizeCache{ Vec2f(100.0f, 100.0f) };

	bool isHovered{ false };

	bool isCapturingMouse{ false };
    Vec2i relativeMouseStartLocation{ Vec2i(0, 0) };
}

// ***********************************************************************

GameView::GameView()
{
    menuName = "Game View";
}

// ***********************************************************************

bool GameView::OnEvent(SDL_Event* event)
{
	if (isHovered)
	{
		if (event->type == SDL_KEYDOWN)
		{
		if (event->key.keysym.scancode == SDL_SCANCODE_TAB && event->key.keysym.mod & KMOD_LSHIFT)
			{
				isCapturingMouse = !isCapturingMouse;
				if (isCapturingMouse)
				{
					SDL_GetGlobalMouseState(&relativeMouseStartLocation.x, &relativeMouseStartLocation.y);
                	SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				else
				{
					SDL_SetRelativeMouseMode(SDL_FALSE);
                	SDL_WarpMouseGlobal(relativeMouseStartLocation.x, relativeMouseStartLocation.y);
				}
			}
		}
		Input::ProcessEvent(event);
		return true;
	}
	return false;
}

// ***********************************************************************

void GameView::Update(Scene& scene, UpdateContext& ctx)
{
	if (!ImGui::Begin("Game", &open))
	{
        isHovered = false;
		Input::ClearHeldState();
		ImGui::End();
		return;
	}

	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 50% opaque whit\

	// TODO: This doesn't trigger if the window is resized from a changing dockspace
	if (windowSizeCache != Vec2f(ImGui::GetContentRegionAvail()))
	{
		// For different aspect ratio modes, use SetCursorPos* functions to size and place the image correctly in the window
		windowSizeCache = ImGui::GetContentRegionAvail();
        if (windowSizeCache.x > 0.0f && windowSizeCache.y > 0.0f)
		{
			gameSizeCache = GameRenderer::GetIdealFrameSize(windowSizeCache.x, windowSizeCache.y);
			GameRenderer::ResizeGameFrame(gameSizeCache.x, gameSizeCache.y);
		}
	}

	float parentAspectRatio = windowSizeCache.x / windowSizeCache.y;
	float gameAspectRatio = gameSizeCache.x / gameSizeCache.y;


	float x = 1.0f, y = x;
    switch (Engine::GetConfig().resolutionStretchMode)
    {
    case ResolutionStretchMode::KeepAspect:
    {
        if (parentAspectRatio < gameAspectRatio)
			y = (1 / gameAspectRatio) * parentAspectRatio;
		else
			x = gameAspectRatio * (1 / parentAspectRatio);
        break;
    }
    case ResolutionStretchMode::KeepWidth:
    {
         if (parentAspectRatio > gameAspectRatio)
            x = gameAspectRatio * (1 / parentAspectRatio);
        break;
    }
    case ResolutionStretchMode::KeepHeight:
    {
        if (parentAspectRatio < gameAspectRatio)
            y = (1 / gameAspectRatio) * parentAspectRatio;
        break;
    }
    default:
        break;
    }

	ImVec2 imageDrawSize = ImVec2(windowSizeCache.x * x, windowSizeCache.y * y);
	ImVec2 imageDrawLocation = ImVec2(ImGui::GetCursorPosX() + windowSizeCache.x * 0.5f - imageDrawSize.x * 0.5f, ImGui::GetCursorPosY() + windowSizeCache.y * 0.5f - imageDrawSize.y * 0.5f);

	ImGui::SetCursorPos(imageDrawLocation);
	ImGui::Image(GfxDevice::GetImGuiTextureID(GameRenderer::GetLastRenderedFrame()), imageDrawSize, uv_min, uv_max);

	if (ImGui::IsItemHovered() || isCapturingMouse)
		isHovered = true;
	else
	{
		Input::ClearHeldState();
		isHovered = false;
	}

	ImGui::End(); // Ending Game

}

// ***********************************************************************

void GameView::OnEditorResize(Vec2f newSize)
{
    windowSizeCache = newSize;
}
