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

void EntityInspector::Update(Scene& scene, UpdateContext& ctx)
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
			if (type.second->isComponent)
			{
				if (ImGui::Selectable(type.first.c_str()))
				{
					Log::Info("Create component of type %s", type.first.c_str());
					scene.Assign(selectedEntity, *type.second);
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

	for (Variant component : ComponentsOnEntity(scene, selectedEntity))
	{
		bool removedComponent = false;
		TypeData& componentType = component.GetType();
		if (ImGui::CollapsingHeader(componentType.name, ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (strstr(componentType.name, "CName") == nullptr && strstr(componentType.name, "CChild") == nullptr && strstr(componentType.name,"CParent") == nullptr)
			{
				ImGui::PushID(componentType.id);
				if (ImGui::Button("Remove Component", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
				{
					scene.Remove(selectedEntity, componentType);
					removedComponent = true;
				}
				ImGui::PopID();
			}
			// #TODO: Ideally systems outside of Scenes shouldn't touch component pools, make something to hide this and ensure safety
			// #TODO: Create a component iterator which gives you variants on each iteration all setup for you

			// Loop through all the members of the component, showing the appropriate UI elements
			for (Member& member : componentType.AsStruct())
			{
				if (member.IsType<float>())
				{
					Variant number = member.Get(component);
					ImGui::DragFloat(member.name, &number.GetValue<float>(), 0.1f);
					member.Set(component, number);
				}
				else if (member.IsType<int>())
				{
					Variant number = member.Get(component);
					ImGui::DragInt(member.name, &number.GetValue<int>(), 0.1f);
					member.Set(component, number);
				}
				else if (member.IsType<Vec2f>())
				{
					Variant var = member.Get(component);
					Vec2f& vec = var.GetValue<Vec2f>();
					float list[2] = { vec.x, vec.y };
					ImGui::DragFloat2(member.name, list, 0.1f);
					vec.x = list[0]; vec.y = list[1];
					member.Set(component, var);
				}
				else if (member.IsType<Vec3f>())
				{
					Variant var = member.Get(component);
					Vec3f& vec = var.GetValue<Vec3f>();
					float list[3] = { vec.x, vec.y, vec.z };
					ImGui::DragFloat3(member.name, list, 0.1f);
					vec.x = list[0]; vec.y = list[1]; vec.z = list[2];
					member.Set(component, var);
				}
				else if (member.IsType<bool>())
				{
					Variant boolean = member.Get(component);
					ImGui::Checkbox(member.name, &boolean.GetValue<bool>());
					member.Set(component, boolean);
				}
				else if (member.IsType<eastl::string>())
				{
					Variant str = member.Get(component);
					ImGui::InputText(member.name, &str.GetValue<eastl::string>());
					member.Set(component, str);
				}
				else if (member.IsType<EntityID>())
				{
					Variant entity = member.Get(component);
					ImGui::Text("{index: %i version: %i}  %s", entity.GetValue<EntityID>().Index(), entity.GetValue<EntityID>().Version(), member.name);
				}
				else if (member.IsType<AssetHandle>())
				{
					Variant handle = member.Get(component);
					eastl::string identifier = AssetDB::GetAssetIdentifier(handle.GetValue<AssetHandle>());
					identifier.set_capacity(200);

					if (ImGui::InputText(member.name, &identifier, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						handle = AssetHandle(identifier);
					}
					member.Set(component, handle);
				}
				else if (member.GetType().castableTo == TypeData::Enum)
				{
					Variant _enum = member.Get(component);
					TypeData_Enum& type = member.GetType().AsEnum();
					int& value = _enum.GetValue<int>();

					if (ImGui::BeginCombo(member.name, type.categories[value].identifier.c_str()))
					{
						for (int i = 0; i < type.categories.size(); i++)
						{
							const Enumerator& enumerator = type.categories[i];
							if (ImGui::Selectable(enumerator.identifier.c_str()))
							{
								value = i;
							}
						}
						ImGui::EndCombo();
					}
					member.Set(component, _enum);
				}
			}
		}
		if (!removedComponent)
			scene.Set(selectedEntity, component);
	}

	ImGui::End();
}
