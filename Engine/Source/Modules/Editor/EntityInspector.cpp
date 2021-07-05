#include "EntityInspector.h"

#include "Log.h"
#include "Scene.h"
#include "AssetDatabase.h"
#include "World.h"
#include "IComponent.h"
#include "Entity.h"

#include <ImGui/imgui.h>
#include <Imgui/misc/cpp/imgui_stdlib.h>

EntityInspector::EntityInspector()
{
    menuName = "Entity Inspector";
}

// ***********************************************************************

void EntityInspector::Update(Scene& scene, UpdateContext& ctx)
{
    Uuid selectedEntity = Editor::GetSelectedEntity();

	ImGui::Begin("Entity Inspector", &open);

	// if (ImGui::Button("Add Component", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
	// 	ImGui::OpenPopup("Select Component");
	// if (ImGui::BeginPopup("Select Component"))
	// {
	// 	ImGui::Text("Aquarium");
	// 	ImGui::Separator();

	// 	for (eastl::pair<eastl::string, TypeData*> type : TypeDatabase::Data::Get().typeNames)
	// 	{
	// 		if (type.second->isComponent)
	// 		{
	// 			if (ImGui::Selectable(type.first.c_str()))
	// 			{
	// 				Log::Info("Create component of type %s", type.first.c_str());
	// 				scene.Assign(selectedEntity, *type.second);
	// 			}
	// 		}
	// 	}
	// 	ImGui::EndPopup();
	// }
    ImGui::NewLine();
    ImGui::NewLine();

	Entity* pEntity = ctx.pWorld->FindEntity(selectedEntity);
	if (pEntity == nullptr)
	{
		ImGui::End();
		return;
	}

	for (IComponent* pComponent : pEntity->GetComponents())
	{
		bool removedComponent = false;
		TypeData& componentType = pComponent->GetTypeData();
		if (ImGui::CollapsingHeader(componentType.name, ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (strstr(componentType.name, "CName") == nullptr && strstr(componentType.name, "CChild") == nullptr && strstr(componentType.name,"CParent") == nullptr)
			{
				ImGui::PushID(componentType.id);
				if (ImGui::Button("Remove Component", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
				{
					pEntity->DestroyComponent(pComponent->GetId());
					removedComponent = true;
				}
				ImGui::PopID();
			}

			// Loop through all the members of the component, showing the appropriate UI elements
			for (Member& member : componentType.AsStruct())
			{
				if (member.IsType<float>())
				{
					Variant number = member.GetFromBase(pComponent);
					ImGui::DragFloat(member.name, &number.GetValue<float>(), 0.1f);
					member.SetOnBase(pComponent, number);
				}
				else if (member.IsType<int>())
				{
					Variant number = member.GetFromBase(pComponent);
					ImGui::DragInt(member.name, &number.GetValue<int>(), 0.1f);
					member.SetOnBase(pComponent, number);
				}
				else if (member.IsType<Vec2f>())
				{
					Variant var = member.GetFromBase(pComponent);
					Vec2f& vec = var.GetValue<Vec2f>();
					float list[2] = { vec.x, vec.y };
					ImGui::DragFloat2(member.name, list, 0.1f);
					vec.x = list[0]; vec.y = list[1];
					member.SetOnBase(pComponent, var);
				}
				else if (member.IsType<Vec3f>())
				{
					Variant var = member.GetFromBase(pComponent);
					Vec3f& vec = var.GetValue<Vec3f>();
					float list[3] = { vec.x, vec.y, vec.z };
					ImGui::DragFloat3(member.name, list, 0.1f);
					vec.x = list[0]; vec.y = list[1]; vec.z = list[2];
					member.SetOnBase(pComponent, var);
				}
				else if (member.IsType<bool>())
				{
					Variant boolean = member.GetFromBase(pComponent);
					ImGui::Checkbox(member.name, &boolean.GetValue<bool>());
					member.SetOnBase(pComponent, boolean);
				}
				else if (member.IsType<eastl::string>())
				{
					Variant str = member.GetFromBase(pComponent);
					ImGui::InputText(member.name, &str.GetValue<eastl::string>());
					member.SetOnBase(pComponent, str);
				}
				else if (member.IsType<EntityID>())
				{
					Variant entity = member.GetFromBase(pComponent);
					ImGui::Text("{index: %i version: %i}  %s", entity.GetValue<EntityID>().Index(), entity.GetValue<EntityID>().Version(), member.name);
				}
				else if (member.IsType<AssetHandle>())
				{
					Variant handle = member.GetFromBase(pComponent);
					eastl::string identifier = AssetDB::GetAssetIdentifier(handle.GetValue<AssetHandle>());
					identifier.set_capacity(200);

					if (ImGui::InputText(member.name, &identifier, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						handle = AssetHandle(identifier);
					}
					member.SetOnBase(pComponent, handle);
				}
				else if (member.GetType().castableTo == TypeData::Enum)
				{
					Variant _enum = member.GetFromBase(pComponent);
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
					member.SetOnBase(pComponent, _enum);
				}
			}
		}
	}

	ImGui::End();
}
