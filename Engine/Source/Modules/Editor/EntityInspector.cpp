#include "EntityInspector.h"

#include "Log.h"
#include "Scene.h"
#include "AssetDatabase.h"

#include <ImGui/imgui.h>
#include <Imgui/misc/cpp/imgui_stdlib.h>

EntityInspector::EntityInspector()
{
    menuName = "Entity Inspector";
}

// ***********************************************************************

void EntityInspector::Update(Scene& scene)
{
    EntityID selectedEntity = Editor::GetSelectedEntity();

	ImGui::Begin("Entity Inspector", &open);

	if (ImGui::Button("Add Component", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
		ImGui::OpenPopup("Select Component");
	if (ImGui::BeginPopup("Select Component"))
	{
		ImGui::Text("Aquarium");
		ImGui::Separator();

		for (eastl::pair<eastl::string, TypeData*> type : TypeDatabase::Data::Get().typeNames)
		{
			if (type.second->pComponentHandler)
			{
				if (ImGui::Selectable(type.first.c_str()))
				{
					Log::Info("Create component of type %s", type.first.c_str());
					type.second->pComponentHandler->Assign(scene, selectedEntity);
				}
			}
		}
		ImGui::EndPopup();
	}
    ImGui::NewLine();
    ImGui::NewLine();


	if (selectedEntity.Index() > scene.entities.size())
	{
		ImGui::End();
		return;
	}
	// We have the entity Id
	for (int i = 0; i < MAX_COMPONENTS; i++)
	{
		// For each component ID, check the bitmask, if no, continue, if yes, save the ID and proceed to access that components data
		eastl::bitset<MAX_COMPONENTS> mask;
		mask.set(i, true);
			
		if (mask == (scene.entities[selectedEntity.Index()].mask & mask))
		{
			TypeData* pComponentType = scene.componentPools[i]->pTypeData;
			if (ImGui::CollapsingHeader(pComponentType->name, ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (strstr(pComponentType->name, "CName") == nullptr && strstr(pComponentType->name, "CChild") == nullptr && strstr(pComponentType->name,"CParent") == nullptr)
				{
					ImGui::PushID(i);
					if (ImGui::Button("Remove Component", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
					{
						pComponentType->pComponentHandler->Remove(scene, selectedEntity);
					}
					ImGui::PopID();
				}
				// #TODO: Ideally systems outside of Scenes shouldn't touch component pools, make something to hide this and ensure safety
				// #TODO: Create a component iterator which gives you variants on each iteration all setup for you

				// Make a new variant to hide this void*
				void* pComponentData = scene.componentPools[i]->get(selectedEntity.Index());

				// Loop through all the members of the component, showing the appropriate UI elements
				for (Member& member : *pComponentType)
				{
					if (member.IsType<float>())
					{
						float* number = member.GetAs<float>(pComponentData);
						ImGui::DragFloat(member.name, number, 0.1f);
					}
					else if (member.IsType<int>())
					{
						int* number = member.GetAs<int>(pComponentData);
						ImGui::DragInt(member.name, number, 0.1f);
					}
					else if (member.IsType<Vec2f>())
					{
						Vec2f& vec = *member.GetAs<Vec2f>(pComponentData);
						float list[2] = { vec.x, vec.y };
						ImGui::DragFloat2(member.name, list, 0.1f);
						vec.x = list[0]; vec.y = list[1];
					}
					else if (member.IsType<Vec3f>())
					{
						Vec3f& vec = *member.GetAs<Vec3f>(pComponentData);
						float list[3] = { vec.x, vec.y, vec.z };
						ImGui::DragFloat3(member.name, list, 0.1f);
						vec.x = list[0]; vec.y = list[1]; vec.z = list[2];
					}
					else if (member.IsType<bool>())
					{
						bool* boolean = member.GetAs<bool>(pComponentData);
						ImGui::Checkbox(member.name, boolean);
					}
					else if (member.IsType<eastl::string>())
					{
						eastl::string* str = member.GetAs<eastl::string>(pComponentData);
						ImGui::InputText(member.name, str);
					}
					else if (member.IsType<EntityID>())
					{
						EntityID& entity = *member.GetAs<EntityID>(pComponentData);
						ImGui::Text("{index: %i version: %i}  %s", entity.Index(), entity.Version(), member.name);
					}
					else if (member.IsType<AssetHandle>())
					{
						AssetHandle* handle = member.GetAs<AssetHandle>(pComponentData);
						eastl::string identifier = AssetDB::GetAssetIdentifier(*handle);
						identifier.set_capacity(200);

						if (ImGui::InputText(member.name, &identifier, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							*handle = AssetHandle(identifier);
						}
					}
				}
			}
		}
	}

	ImGui::End();
}
