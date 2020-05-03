#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"
#include "Scene.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Profiler.h"
#include "Engine.h"

#include <Imgui/imgui.h>
#include <Imgui/misc/cpp/imgui_stdlib.h>

#include <SDL_scancode.h>

namespace {
	bool showEditor = true;
	bool showLog = true;
	bool showEntityInspector = true;
	bool showEntityList = true;
	bool showFrameStats = true;
	bool showImGuiDemo = false;
	EntityID selectedEntity = INVALID_ENTITY;

	int frameStatsCounter = 0; // used so we only update framerate every few frames to make it less annoying to read
	double oldRealFrameTime;
	double oldObservedFrameTime;
}

struct LogApp
{
    ImGuiTextFilter     filter;
    bool                scrollToBottom{ true };

    void Draw()
    {
        ImGui::Begin("Log", &showLog);

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
};

void ShowLog()
{
	// This isn't ideal, put it inside an editor state struct or something
	static LogApp log;

	if (!showLog)
		return;

	log.Draw();
}

void ShowEntityInspector(Scene& scene)
{
	if (!showEntityInspector)
		return;

	ImGui::Begin("Entity Inspector", &showEntityInspector);

	if (GetEntityIndex(selectedEntity) > scene.entities.size())
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
			
		if (mask == (scene.entities[GetEntityIndex(selectedEntity)].mask & mask))
		{
			TypeData* pComponentType = scene.componentPools[i]->pTypeData;
			if (ImGui::CollapsingHeader(pComponentType->name, ImGuiTreeNodeFlags_DefaultOpen))
			{
				// #TODO: Ideally systems outside of Scenes shouldn't touch component pools, make something to hide this and ensure safety
				// #TODO: Create a component iterator which gives you variants on each iteration all setup for you

				// Make a new variant to hide this void*
				void* pComponentData = scene.componentPools[i]->get(GetEntityIndex(selectedEntity));

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
						eastl::string str = *(member.GetAs<eastl::string>(pComponentData));
						ImGui::InputText(member.name, str.data(), str.size());
					}
					else if (member.IsType<EntityID>())
					{
						EntityID& entity = *member.GetAs<EntityID>(pComponentData);
						ImGui::Text("{index: %i version: %i}  %s", GetEntityIndex(entity), GetEntityVersion(entity), member.name);
					}
				}
			}
		}
	}

	ImGui::End();
}

void RecurseDrawEntityTree(Scene& scene, EntityID parent)
{
	CParent* pParent = scene.Get<CParent>(parent);
			
	EntityID currChild = pParent->firstChild;
	for(int i = 0; i < pParent->nChildren; i++)
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		
		if (!scene.Has<CParent>(currChild))
			nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)currChild, nodeFlags, "%i - %s", GetEntityIndex(currChild), scene.GetEntityName(currChild).c_str());
		if (ImGui::IsItemClicked())
			selectedEntity = currChild;
		
		if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
		{
			if (scene.Has<CParent>(currChild))
				RecurseDrawEntityTree(scene, currChild);

			ImGui::TreePop();
		}
		currChild = scene.Get<CChild>(currChild)->next;
	}
}

void ShowEntityList(Scene& scene)
{
	if (!showEntityList)
		return;

	ImGui::Begin("Entity Editor", &showEntityList);

	for (EntityID entity : SceneView<>(scene))
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

		if (selectedEntity == entity)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (!scene.Has<CParent>(entity))
			nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (scene.Has<CChild>(entity))
			continue;

		bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)entity, nodeFlags, "%i - %s", GetEntityIndex(entity), scene.GetEntityName(entity).c_str());
		if (ImGui::IsItemClicked())
			selectedEntity = entity;
		
		if (nodeOpened && !(nodeFlags & ImGuiTreeNodeFlags_Leaf))
		{
			if (scene.Has<CParent>(entity))
				RecurseDrawEntityTree(scene, entity);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void ShowFrameStats()
{
	if (!showFrameStats)
		return;

	double realFrameTime;
	double observedFrameTime;
	Engine::GetFrameRates(realFrameTime, observedFrameTime);

	if (++frameStatsCounter > 30)
	{
		oldRealFrameTime = realFrameTime;
		oldObservedFrameTime = observedFrameTime;
		frameStatsCounter = 0;
	}

	ImGui::Begin("Frame Stats", &showFrameStats);

	ImGui::Text("Real frame time %.6f ms/frame (%.3f FPS)", oldRealFrameTime * 1000.0, 1.0 / oldRealFrameTime);
	ImGui::Text("Observed frame time %.6f ms/frame (%.3f FPS)", oldObservedFrameTime * 1000.0, 1.0 / oldObservedFrameTime);

	ImGui::Separator();

	int elements = 0;
	Profiler::ScopeData* pFrameData = nullptr;
	Profiler::GetFrameData(&pFrameData, elements);
	for (size_t i = 0; i < elements; ++i)
	{
		// Might want to add some smoothing and history to this data? Can be noisey, especially for functions not called every frame
		// Also maybe sort so we can see most expensive things at the top? Lots of expansion possibility here really
		double inMs = pFrameData[i].time * 1000.0;
		eastl::string str;
		str.sprintf("%s - %fms/frame", pFrameData[i].name, inMs);
		ImGui::Text(str.c_str());
	}

	ImGui::End();
}

void Editor::OnFrame(Scene& scene, float /* deltaTime */)
{
	if (Input::GetKeyDown(SDL_SCANCODE_F8))
		showEditor = !showEditor;

	if (!showEditor)
		return;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Show Imgui Demo")) { showImGuiDemo = !showImGuiDemo; }
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) { Engine::StartShutdown(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Editors"))
		{
			if (ImGui::MenuItem("Entity List")) { showEntityList = !showEntityList; }
			if (ImGui::MenuItem("Entity Inspector")) { showEntityInspector = !showEntityInspector; }
			if (ImGui::MenuItem("Console")) { showLog = !showLog; }
			if (ImGui::MenuItem("FrameStats")) { showFrameStats = !showFrameStats; }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ShowLog();
	ShowEntityInspector(scene);
	ShowEntityList(scene);
	ShowFrameStats();
	if (showImGuiDemo)
		ImGui::ShowDemoWindow();
}