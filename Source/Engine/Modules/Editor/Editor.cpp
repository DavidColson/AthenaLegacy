#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"
#include "Scene.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Profiler.h"
#include "Engine.h"
#include "SceneSerializer.h"
#include "Json.h"
#include "FileSystem.h"
#include "AssetDatabase.h"

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
	EntityID selectedEntity = EntityID::InvalidID();

	eastl::string levelSaveModalFilename;
	eastl::vector<eastl::string> levelOpenModalFiles;
	int selectedLevelFile = -1;

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

	if (ImGui::Button("Add Entity", Vec2f(ImGui::GetContentRegionAvailWidth(), 0.0f)))
	{
		scene.NewEntity("Entity");
	}

	for (EntityID entity : SceneView<>(scene))
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

		if (selectedEntity == entity)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (!scene.Has<CParent>(entity))
			nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (scene.Has<CChild>(entity))
			continue;

		bool nodeOpened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.value, nodeFlags, "%i - %s", entity.Index(), scene.GetEntityName(entity).c_str());
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

	bool bSaveModal = false;
	bool bOpenModal = false;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save Level"))
           		bSaveModal = true;
			if (ImGui::MenuItem("Open Level"))
				bOpenModal = true;
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

	if (bSaveModal)
	{
		levelSaveModalFilename.set_capacity(100);
		ImGui::OpenPopup("Save Level As");
	}
	if (ImGui::BeginPopupModal("Save Level As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Choose a filename\n\n");
		ImGui::Separator();
		
		ImGui::Text("Filename: ");
		ImGui::SameLine();
		ImGui::InputText("", &levelSaveModalFilename);
		ImGui::SameLine();
		ImGui::Text(".lvl");

		if (ImGui::Button("Save", ImVec2(120, 0))) 
		{
			eastl::string filePath;
			filePath.sprintf("Resources/Levels/%s.lvl", levelSaveModalFilename.c_str());
			JsonValue jsonScene = SceneSerializer::ToJson(scene);
			FileSys::WriteWholeFile(filePath, SerializeJsonValue(jsonScene));
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (bOpenModal)
	{
		ImGui::OpenPopup("Open Level");

		levelOpenModalFiles.clear();
		eastl::vector<Path> levels = FileSys::ListFiles("Resources/Levels/");
		for (Path& levelName : levels)
		{
			levelOpenModalFiles.push_back(levelName.AsString());
		}
	}
	if (ImGui::BeginPopupModal("Open Level", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Choose a file\n");
		ImGui::Separator();
		
		ImGui::BeginChild("Levels", ImVec2(0.0f, 200.0f), true);
		for (int i = 0; i < levelOpenModalFiles.size(); i++)
		{
			if (ImGui::Selectable(levelOpenModalFiles[i].c_str(), i == selectedLevelFile))
				selectedLevelFile = i;
		}
		ImGui::EndChild();

		if (ImGui::Button("Open", ImVec2(120, 0))) 
		{
			JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile(levelOpenModalFiles[selectedLevelFile]));
			Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);
			Engine::SetActiveScene(pScene);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ShowLog();
	ShowEntityInspector(scene);
	ShowEntityList(scene);
	ShowFrameStats();
	if (showImGuiDemo)
		ImGui::ShowDemoWindow();
}