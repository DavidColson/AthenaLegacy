#include "GameView.h"

#include "Rendering/GameRenderer.h"
#include "GraphicsDevice.h"
#include "Engine.h"

#include <ImGui/imgui.h>


namespace
{
	Vec2f windowSizeCache{ Vec2f(100.0f, 100.0f) };
	Vec2f gameSizeCache{ Vec2f(100.0f, 100.0f) };
}

// ***********************************************************************

GameView::GameView()
{
    menuName = "Game View";
}

// ***********************************************************************

void GameView::Update(Scene& scene, float deltaTime)
{
	if (!ImGui::Begin("Game", &open))
	{
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
			GameRenderer::ResizeGameFrame(scene, gameSizeCache.x, gameSizeCache.y);
		}
	}

	float parentAspectRatio = windowSizeCache.x / windowSizeCache.y;
	float gameAspectRatio = gameSizeCache.x / gameSizeCache.y;


	float x = 1.0f, y = x;
    switch (Engine::GetConfig().resolutionStretchMode)
    {
    case 2:
    {
        if (parentAspectRatio < gameAspectRatio)
			y = (1 / gameAspectRatio) * parentAspectRatio;
		else
			x = gameAspectRatio * (1 / parentAspectRatio);
        break;
    }
    case 3:
    {
         if (parentAspectRatio > gameAspectRatio)
            x = gameAspectRatio * (1 / parentAspectRatio);
        break;
    }
    case 4:
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

	if (ImGui::IsItemHovered())
	{
		ImGui::CaptureMouseFromApp(false);
		ImGui::CaptureKeyboardFromApp(false);
	}

	ImGui::End(); // Ending Game

}

// ***********************************************************************

void GameView::OnEditorResize(Vec2f newSize)
{
    windowSizeCache = newSize;
}
