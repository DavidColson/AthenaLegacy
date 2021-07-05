#include "SceneHeirarchy.h"

#include "Scene.h"
#include "World.h"
#include "Entity.h"

#include <ImGui/imgui.h>

SceneHeirarchy::SceneHeirarchy()
{
    menuName = "Scene Heirarchy";
}

// ***********************************************************************

void RecurseDrawEntityTree(UpdateContext& ctx, EntityID parent)
{
	// New entity system doesn't support entity parenting yet. Come back to this

	// CParent* pParent = scene.Get<CParent>(parent);
			
	// EntityID currChild = pParent->firstChild;
	// for(int i = 0; i < pParent->nChildren; i++)
	// {
	// 	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		
	// 	if (!scene.Has<CParent>(currChild))
	// 		nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	// 	bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)currChild.value, nodeFlags, "%i - %s", currChild.Index(), scene.GetEntityName(currChild).c_str());
	// 	if (ImGui::IsItemClicked())
	// 		Editor::SetSelectedEntity(currChild);
		
	// 	if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
	// 	{
	// 		if (scene.Has<CParent>(currChild))
	// 			RecurseDrawEntityTree(scene, currChild);

	// 		ImGui::TreePop();
	// 	}
	// 	currChild = scene.Get<CChild>(currChild)->next;
	// }
}

// ***********************************************************************

void SceneHeirarchy::Update(Scene& scene, UpdateContext& ctx)
{
	ImGui::Begin("Scene Heirarchy", &open);

	if (ImGui::Button("Add Entity", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
	{
		scene.NewEntity("Entity");
	}

	uint64_t counter = 0;
	for (Entity* pEntity : (*ctx.pWorld))
	{
		counter++;
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

		if (Editor::GetSelectedEntity() == pEntity->GetId())
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		// if (!scene.Has<CParent>(entity))
		nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		// if (scene.Has<CChild>(entity))
		// 	continue;

		bool nodeOpened = ImGui::TreeNodeEx((void*)counter, nodeFlags, "%s", pEntity->name.c_str());
		if (ImGui::IsItemClicked())
			Editor::SetSelectedEntity(pEntity->GetId());
		
		if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
		{
			// if (scene.Has<CParent>(entity))
			// 	RecurseDrawEntityTree(scene, entity);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
