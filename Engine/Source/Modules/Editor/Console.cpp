#include "Console.h"

#include "Log.h"

#include <ImGui/imgui.h>

namespace
{
    ImGuiTextFilter     filter;
}

// ***********************************************************************

Console::Console()
{
    menuName = "Console";
}

// ***********************************************************************

void Console::Update(Scene& scene)
{
	ImGui::Begin("Log", &open);

		ImGui::Checkbox("Scroll To Bottom", &scrollToBottom);
		ImGui::SameLine();
		ImGui::Button("Clear");
		ImGui::SameLine();
		filter.Draw("Filter", -100.0f);
		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		for (const Log::LogEntry& entry : Log::GetLogHistory())
		{
			const char* item = entry.message.c_str();
			if (filter.PassFilter(item, nullptr))
			{
				ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_Text);;
				switch (entry.level)
				{
				case Log::ECrit:
					col = ImColor(1.0f, 0.0f, 0.0f); break;
				case Log::EWarn:
					col = ImColor(1.0f, 1.0f, 0.0f); break;
				default: break;
				}

				ImGui::PushStyleColor(ImGuiCol_Text, col);
				ImGui::TextUnformatted(item);
            	ImGui::PopStyleColor();
			}
		}
		
		if (scrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();

		ImGui::End();
}