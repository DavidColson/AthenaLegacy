#include "GameView.h"

#include "Rendering/GameRenderer.h"
#include "GraphicsDevice.h"

#include <ImGui/imgui.h>

// ***********************************************************************

GameView::GameView()
{
    menuName = "Game View";
}

// ***********************************************************************

void GameView::Update(Scene& scene)
{
	ImGui::Begin("Game", &open);

	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 50% opaque whit\

	// TODO: This doesn't trigger if the window is resized from a changing dockspace
	if (windowSizeCache != Vec2f(ImGui::GetContentRegionAvail()))
	{
		windowSizeCache = ImGui::GetContentRegionAvail();
        if (windowSizeCache.x > 0.0f && windowSizeCache.y > 0.0f)
		    GameRenderer::ResizeGameFrame(scene, windowSizeCache.x, windowSizeCache.y);
	}

	ImGui::Image(GfxDevice::GetImGuiTextureID(GameRenderer::GetLastRenderedFrame()), ImVec2(windowSizeCache.x, windowSizeCache.y), uv_min, uv_max);

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
