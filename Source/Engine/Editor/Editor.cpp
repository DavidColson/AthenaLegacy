#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"
#include "GameFramework/World.h"
#include "Maths/Vec3.h"
#include "Maths/Vec2.h"
#include "Profiler.h"

#include <vector>
#include <string>
#include <ThirdParty/Imgui/imgui.h>

namespace {
	bool showEditor = true;
	bool showLog = true;
	bool showEntityInspector = true;
	bool showEntityList = true;
	bool showFrameStats = true;
	EntityID selectedEntity = -1;

	int frameStatsCounter = 0; // used so we only update framerate every few frames to make it less annoying to read
	double oldRealFrameTime;
	double oldObservedFrameTime;
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

void ShowEntityInspector(Scene& scene)
{
	if (!showEntityInspector)
		return;

	ImGui::Begin("Entity Inspector", &showEntityInspector);

	if (GetEntityIndex(selectedEntity) > scene.m_entities.size())
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
			
		if (mask == (scene.m_entities[GetEntityIndex(selectedEntity)].m_mask & mask))
		{
			TypeData* pComponentType = scene.m_componentPools[i]->pTypeData;
			if (ImGui::CollapsingHeader(pComponentType->m_name))
			{
				// #TODO: Ideally systems outside of Scenes shouldn't touch component pools, make something to hide this and ensure safety
				// #TODO: Create a component iterator which gives you variants on each iteration all setup for you

				// Make a new variant to hide this void*
				void* pComponentData = scene.m_componentPools[i]->get(GetEntityIndex(selectedEntity));

				// Loop through all the members of the component, showing the appropriate UI elements
				for (Member& member : *pComponentType)
				{
					if (member.IsType<float>())
					{
						float* number = member.Get<float>(pComponentData);
						ImGui::DragFloat(member.m_name, number, 0.1f);
					}
					else if (member.IsType<int>())
					{
						int* number = member.Get<int>(pComponentData);
						ImGui::DragInt(member.m_name, number, 0.1f);
					}
					else if (member.IsType<Vec2f>())
					{
						Vec2f& vec = *member.Get<Vec2f>(pComponentData);
						float list[2] = { vec.x, vec.y };
						ImGui::DragFloat2(member.m_name, list, 0.1f);
						vec.x = list[0]; vec.y = list[1];
					}
					else if (member.IsType<Vec3f>())
					{
						Vec3f& vec = *member.Get<Vec3f>(pComponentData);
						float list[3] = { vec.x, vec.y, vec.z };
						ImGui::DragFloat3(member.m_name, list, 0.1f);
						vec.x = list[0]; vec.y = list[1]; vec.z = list[2];
					}
					else if (member.IsType<bool>())
					{
						bool* boolean = member.Get<bool>(pComponentData);
						ImGui::Checkbox(member.m_name, boolean);
					}
					else if (member.IsType<EntityID>())
					{
						EntityID& entity = *member.Get<EntityID>(pComponentData);
						ImGui::Text("{index: %i version: %i}  %s", GetEntityIndex(entity), GetEntityVersion(entity), member.m_name);
					}
				}
			}
		}
	}

	ImGui::End();
}

void ShowEntityList(Scene& scene)
{
	if (!showEntityList)
		return;

	ImGui::Begin("Entity Editor", &showEntityList);

	for (EntityID entity : SceneView<>(scene))
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (selectedEntity == entity)
			node_flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::TreeNodeEx((void*)(uintptr_t)entity, node_flags, "%i - %s", GetEntityIndex(entity), scene.GetEntityName(entity));
		if (ImGui::IsItemClicked())
			selectedEntity = entity;
	}

	ImGui::End();
}

void ShowFrameStats(double realFrameTime, double observedFrameTime)
{
	if (!showFrameStats)
		return;

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

  std::vector<Profiler::ScopeData>& data = Profiler::GetFrameData();
  for (size_t i = 0; i < data.size(); ++i)
  {
  	// Might want to add some smoothing and history to this data? Can be noisey, especially for functions not called every frame
  	// Also maybe sort so we can see most expensive things at the top? Lots of expansion possibility here really
  	double inMs = data[i].m_time * 1000.0;
  	ImGui::Text(StringFormat("%s - %fms/frame", data[i].m_name, inMs).c_str());
  }

	ImGui::End();
}

void Editor::ShowEditor(Scene& scene, bool& shutdown, double realFrameTime, double observedFrameTime)
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
			if (ImGui::MenuItem("FrameStats")) { showFrameStats = !showFrameStats; }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ShowLog();
	ShowEntityInspector(scene);
	ShowEntityList(scene);
	ShowFrameStats(realFrameTime, observedFrameTime);
	//ImGui::ShowDemoWindow();
}