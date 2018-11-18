#include "Editor.h"

#include "Input/Input.h"

#include <ThirdParty/Imgui/imgui.h>

bool showEditor = true;

void Editor::ShowEditor(bool& shutdown)
{
	if (Input::GetKeyDown(SDL_SCANCODE_F8))
		showEditor = !showEditor;

	if (!showEditor)
		return;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Pause")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) { shutdown = true; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Editors"))
		{
			if (ImGui::MenuItem("Console", "CTRL+Z")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
