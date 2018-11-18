#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"

#include <vector>
#include <string>
#include <ThirdParty/Imgui/imgui.h>

bool showEditor = true;
bool showLog = true;

void ShowLog()
{
	if (!showLog)
		return;

	ImGui::Begin("Log", &showLog);

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	std::vector<std::string> entries = Log::GetLogHistory();
	for (int i = 0; i < entries.size(); i++)
	{
		const char* item = entries[i].c_str();

		ImGui::TextUnformatted(item);
	}
	ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();

	ImGui::End();
}

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
			if (ImGui::MenuItem("Console")) { showLog = !showLog; }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ShowLog();
}


