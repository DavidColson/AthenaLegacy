#include "SceneHeirarchy.h"

#include "Scene.h"

#include <ImGui/imgui.h>

SceneHeirarchy::SceneHeirarchy()
{
    menuName = "Scene Heirarchy";
}

// ***********************************************************************

void RecurseDrawEntityTree(Scene& scene, EntityID parent)
{
	CParent* pParent = scene.Get<CParent>(parent);
			
	EntityID currChild = pParent->firstChild;
	for(int i = 0; i < pParent->nChildren; i++)
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		
		if (!scene.Has<CParent>(currChild))
			nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)currChild.value, nodeFlags, "%i - %s", currChild.Index(), scene.GetEntityName(currChild).c_str());
		if (ImGui::IsItemClicked())
			Editor::SetSelectedEntity(currChild);
		
		if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
		{
			if (scene.Has<CParent>(currChild))
				RecurseDrawEntityTree(scene, currChild);

			ImGui::TreePop();
		}
		currChild = scene.Get<CChild>(currChild)->next;
	}
}

// ***********************************************************************

void SceneHeirarchy::Update(Scene& scene, float deltaTime)
{
	ImGui::Begin("Entity Editor", &open);

	if (ImGui::Button("Add Entity", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
	{
		scene.NewEntity("Entity");
	}

	for (EntityID entity : SceneIterator<>(scene))
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

		if (Editor::GetSelectedEntity() == entity)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (!scene.Has<CParent>(entity))
			nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (scene.Has<CChild>(entity))
			continue;

		bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.value, nodeFlags, "%i - %s", entity.Index(), scene.GetEntityName(entity).c_str());
		if (ImGui::IsItemClicked())
			Editor::SetSelectedEntity(entity);
		
		if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
		{
			if (scene.Has<CParent>(entity))
				RecurseDrawEntityTree(scene, entity);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
