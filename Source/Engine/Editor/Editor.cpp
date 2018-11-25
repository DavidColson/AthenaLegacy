#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"
#include "GameFramework/World.h"
#include "Maths/Maths.h"

#include <vector>
#include <string>
#include <ThirdParty/Imgui/imgui.h>

namespace {
	bool showEditor = true;
	bool showLog = false;
	bool showEntityInspector = true;
	bool showEntityList = true;
	EntityID selectedEntity = -1;
	static Space* pCurrentSpace;
}

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

void ShowEntityInspector()
{
	if (!showEntityInspector)
		return;

	ImGui::Begin("Entity Inspector", &showEntityInspector);

	if (selectedEntity > pCurrentSpace->m_entities.size())
	{
		ImGui::End();
		return;
	}
	// We have the entity Id
	for (int i = 0; i < MAX_COMPONENTS; i++)
	{
		// For each component ID, check the bitmask, if no, continue, if yes, save the ID and proceed to access that components data
		std::bitset<MAX_COMPONENTS> mask;
		mask.set(i, true);
			
		if (mask == (pCurrentSpace->m_entities[selectedEntity] & mask))
		{
			// Lookup the type object for that component ID (need a new accessor in TypeDB)
			Type* componentType = TypeDB::GetType(g_componentTypeMap.LookupTypeId(i));
			if (ImGui::CollapsingHeader(componentType->m_name))
			{
				// Directly access componentPools and put the pointer to that component in a RefVariant's m_data, and save the type as well
				VariantBase component;
				component.m_type = componentType;
				component.m_data = pCurrentSpace->m_componentPools[i]->get(selectedEntity);

				// Loop the memberlist of the type, creating editors for each type, getting from the RefVariant of the component
				for (std::pair<std::string, TypeDB::Member*> member : component.m_type->m_memberList)
				{
					if (member.second->m_type == TypeDB::GetType<float>())
					{
						float* number = (float*)member.second->GetRefValue(component).m_data;
						ImGui::DragFloat(member.first.c_str(), number, 0.1f);
					}
					else if (member.second->m_type == TypeDB::GetType<vec2>())
					{
						vec2* vec = (vec2*)member.second->GetRefValue(component).m_data;
						float list[2] = { vec->x, vec->y };
						ImGui::DragFloat2(member.first.c_str(), list, 0.1f);
						vec->x = list[0]; vec->y = list[1];
					}
					else if (member.second->m_type == TypeDB::GetType<vec3>())
					{
						vec3* vec = (vec3*)member.second->GetRefValue(component).m_data;
						float list[3] = { vec->x, vec->y, vec->z };
						ImGui::DragFloat3(member.first.c_str(), list, 0.1f);
						vec->x = list[0]; vec->y = list[1]; vec->z = list[2];
					}
				}
			}
		}
	}

	ImGui::End();
}

void ShowEntityList()
{
	if (!showEntityList)
		return;

	ImGui::Begin("Entity List", &showEntityList);

	// We have the entity Id
	int entity = 1;
	for (int i = 0; i < pCurrentSpace->m_entities.size(); i++)
	{
		EntityID entity = i;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (selectedEntity == entity)
			node_flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::TreeNodeEx((void*)(uintptr_t)entity, node_flags, "Entity(%i)", entity);
		if (ImGui::IsItemClicked())
			selectedEntity = entity;
	}
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
			if (ImGui::MenuItem("Entity List")) { showEntityList = !showEntityList; }
			if (ImGui::MenuItem("Entity Inspector")) { showEntityInspector = !showEntityInspector; }
			if (ImGui::MenuItem("Console")) { showLog = !showLog; }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ShowLog();
	ShowEntityInspector();
	ShowEntityList();
}

void Editor::SetCurrentSpace(Space* pSpace)
{
	pCurrentSpace = pSpace;
}
