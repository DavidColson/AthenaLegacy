#include "Editor.h"

#include "Input/Input.h"
#include "Log.h"
#include "Scene.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Engine.h"
#include "SceneSerializer.h"
#include "Json.h"
#include "FileSystem.h"
#include "AssetDatabase.h"
#include "GraphicsDevice.h"
#include "AppWindow.h"
#include "Rendering/GameRenderer.h"

#include "EntityInspector.h"
#include "FrameStats.h"
#include "SceneHeirarchy.h"
#include "GameView.h"
#include "Console.h"
#include "SceneView.h"

#include <SDL.h>
#include <Imgui/imgui.h>
#include <Imgui/misc/cpp/imgui_stdlib.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>
#include <EASTL/unique_ptr.h>


namespace {
	bool showEditor = false;

	EntityID selectedEntity = EntityID::InvalidID();

	eastl::string levelSaveModalFilename;
	eastl::vector<eastl::string> levelOpenModalFiles;
	int selectedLevelFile = -1;

    RenderTargetHandle editorRenderTarget;

	eastl::vector<eastl::unique_ptr<EditorTool>> tools;
}

// ***********************************************************************

struct ImGuiDemoTool : public EditorTool
{
	ImGuiDemoTool() { menuName = "Imgui Demo"; open = false; }
	virtual void Update(Scene& scene, float deltaTime) override { ImGui::ShowDemoWindow(&open); }
};

// ***********************************************************************

EntityID Editor::GetSelectedEntity()
{
	return selectedEntity;
}

// ***********************************************************************

void Editor::SetSelectedEntity(EntityID entity)
{
	selectedEntity = entity;
}

// ***********************************************************************

void Editor::Initialize(bool enabled)
{
	showEditor = enabled;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui_ImplSDL2_InitForD3D(AppWindow::GetSDLWindow());

	// One day imgui should just use the graphics device API and not need to do this
	GfxDevice::InitImgui();

	io.Fonts->AddFontFromFileTTF("Engine/Source/ThirdParty/Imgui/misc/fonts/Roboto-Medium.ttf", 13.0f);
	ImGui::StyleColorsDark();

    editorRenderTarget = GfxDevice::CreateRenderTarget(AppWindow::GetWidth(), AppWindow::GetHeight(), 1, "Editor Render Target");

	tools.push_back(eastl::make_unique<FrameStats>());
	tools.push_back(eastl::make_unique<EntityInspector>());
	tools.push_back(eastl::make_unique<SceneHeirarchy>());
	tools.push_back(eastl::make_unique<GameView>());
	tools.push_back(eastl::make_unique<SceneView>());
	tools.push_back(eastl::make_unique<Console>());
	tools.push_back(eastl::make_unique<ImGuiDemoTool>());
}

// ***********************************************************************

void Editor::ProcessEvent(Scene& scene, SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);
	switch (event->type)
	{
	case SDL_KEYDOWN:
		if (event->key.keysym.scancode == SDL_SCANCODE_F8)
		{
			Editor::ToggleEditor();

			AppWindow::Resize(AppWindow::GetWidth(), AppWindow::GetHeight());
			// When leaving editor mode, tell the game to resize itself to full screen, otherwise resize ourselves
			if (!Editor::IsActive())
				GameRenderer::ResizeGameFrame(scene, AppWindow::GetWidth(), AppWindow::GetHeight());
			else
				Editor::ResizeEditorFrame(AppWindow::GetWidth(), AppWindow::GetHeight());
		}
		break;
	default:
		break;
	}
}

// ***********************************************************************

void Editor::PreUpdate()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(AppWindow::GetSDLWindow());
	ImGui::NewFrame();
}

// ***********************************************************************

TextureHandle Editor::DrawFrame(Scene& scene, float deltaTime)
{
	if (!showEditor)
	{
		// Need to run this code even when editor is disabled to correctly update the imgui viewports
		ImGui::Render();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		return INVALID_HANDLE;
	}

	// Setup dockspace
	// ---------------

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;;

	bool open = true;
	ImGui::Begin("MainDockspaceWindow", &open, window_flags);

	ImGui::PopStyleVar(3);
	
	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);


	// Create main menu bar
	// --------------------

	bool bSaveModal = false;
	bool bOpenModal = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save Level"))
           		bSaveModal = true;
			if (ImGui::MenuItem("Open Level"))
				bOpenModal = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) { Engine::StartShutdown(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Editors"))
		{
			for (eastl::unique_ptr<EditorTool>& tool : tools)
			{
				if (ImGui::MenuItem(tool->menuName.c_str()))
					tool->open = !tool->open;
			}
			
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::End(); // Ending MainDockspaceWindow


	// Create level save and load modals
	// ---------------------------

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
			Path basePath(Engine::GetConfig().gameResourcesPath);
			eastl::string filePath;
			filePath.sprintf("Levels/%s.lvl", levelSaveModalFilename.c_str());
			JsonValue jsonScene = SceneSerializer::ToJson(scene);
			FileSys::WriteWholeFile(basePath / filePath, SerializeJsonValue(jsonScene));
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

		Path basePath(Engine::GetConfig().gameResourcesPath);
		levelOpenModalFiles.clear();
		eastl::vector<Path> levels = FileSys::ListFiles(basePath / "Levels/");
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
			if (ImGui::Selectable(Path(levelOpenModalFiles[i]).Filename().AsRawString(), i == selectedLevelFile))
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

	// Update/Draw editor tools
	// ------------------------

	for (eastl::unique_ptr<EditorTool>& tool : tools)
	{
		if (tool->open)
			tool->Update(scene, deltaTime);
	}

	// Render editor to our render target
	// ----------------------------------

	GfxDevice::BindRenderTarget(editorRenderTarget);
	GfxDevice::ClearRenderTarget(editorRenderTarget, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
	GfxDevice::SetViewport(0.0f, 0.0f, AppWindow::GetWidth(), AppWindow::GetHeight());
	{			
		GFX_SCOPED_EVENT("Drawing imgui");
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
	GfxDevice::UnbindRenderTarget(editorRenderTarget);

	return GfxDevice::GetTexture(editorRenderTarget);
}

// ***********************************************************************

void Editor::Destroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
}

// ***********************************************************************

void Editor::ResizeEditorFrame(float width, float height)
{
	GfxDevice::FreeRenderTarget(editorRenderTarget);
    editorRenderTarget = GfxDevice::CreateRenderTarget(width, height, 1, "Editor Render Target");

	for (eastl::unique_ptr<EditorTool>& tool : tools)
	{
		if (tool->open)
			tool->OnEditorResize(Vec2f(width, height));
	}
}

// ***********************************************************************

bool Editor::IsActive()
{
	return showEditor;
}

// ***********************************************************************

void Editor::ToggleEditor()
{
	showEditor = !showEditor;

	ImGuiIO &io = ImGui::GetIO();
	if (showEditor)
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	else
		io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
}