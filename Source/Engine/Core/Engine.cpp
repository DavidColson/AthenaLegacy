#include "Engine.h"

#include <SDL.h>
#include <Imgui/imgui.h> 
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "AppWindow.h"
#include "GraphicsDevice.h"
#include "Rendering/RenderSystem.h"
#include "AudioDevice.h"
#include "Scene.h"
#include "Input/Input.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "Profiler.h"
#include "Memory.h"
#include "Maths.h"
#include "Matrix.h"
#include "Quat.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	bool g_gameRunning{ true };

	void (*pSceneCallBack)(Scene& scene);

	Scene* pCurrentScene{ nullptr };
	Scene* pPendingSceneLoad{ nullptr };
	SDL_Window* g_pWindow{ nullptr };

	RenderTargetHandle editorRenderTarget;
}

void Engine::GetFrameRates(double& outReal, double& outLimited)
{
	outReal = g_realFrameTime;
	outLimited = g_observedFrameTime;
}

void Engine::StartShutdown()
{
	g_gameRunning = false;
}

void Engine::NewSceneCreated(Scene& scene)
{
	RenderSystem::OnSceneCreate(scene);

	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, TransformHeirarchy);

	// @Improvement consider allowing engine config files to change the update order of systems
}

void Engine::SetActiveScene(Scene* pScene)
{
	pPendingSceneLoad = pScene;
}

void Engine::SetSceneCreateCallback(void (*pCallBackFunc)(Scene&))
{
	pSceneCallBack = pCallBackFunc;
}

void Engine::Initialize()
{
	// Startup flow
	Log::Info("Engine starting up");
	
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	AppWindow::Create(1800.f, 1000.f);

	Log::SetLogLevel(Log::EDebug);

	RenderSystem::Initialize(1800.0f, 1000.0f);
	AudioDevice::Initialize();
	Input::CreateInputState();	


	// TEMP
    editorRenderTarget = GfxDevice::CreateRenderTarget(AppWindow::GetWidth(), AppWindow::GetHeight(), 1, "Editor Render Target");
}

void Engine::Run(Scene *pScene)
{	
	pCurrentScene = pScene;

	if (pSceneCallBack)
		pSceneCallBack(*pCurrentScene);

	// Game update loop
	double frameTime = 0.016f;
	double targetFrameTime = 0.0166f;
	while (g_gameRunning)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		AssetDB::UpdateHotReloading();

		// Deal with events
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					if (event.window.windowID == SDL_GetWindowID(g_pWindow))
						AppWindow::Resize((float)event.window.data1, (float)event.window.data2);
					break;
				default:
					break;
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					Editor::ToggleEditor();
					if (!Editor::IsInEditor())
					{
						RenderSystem::ResizeGameFrame(*pCurrentScene, AppWindow::GetWidth(), AppWindow::GetHeight());
					}
				}
				break;
			case SDL_QUIT:
				Engine::StartShutdown();
				break;
			}
		}

		if (Editor::IsInEditor())
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplSDL2_NewFrame(AppWindow::GetSDLWindow());
			ImGui::NewFrame();
		}

		// Simulate current game scene
		pCurrentScene->SimulateScene((float)frameTime);
		Profiler::ClearFrameData();

		// Render the game
		RenderSystem::OnFrame(*pCurrentScene, (float)frameTime);

		// If in editor, render the editor
		if (Editor::IsInEditor())
		{
			Editor::SetGameFrame(RenderSystem::GetGameFrame());
			Editor::OnFrame(*pCurrentScene, (float)frameTime);

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
			Editor::FreeGameFrame();

			AppWindow::RenderToWindow(GfxDevice::GetTexture(editorRenderTarget));
		}
		else
		{
			AppWindow::RenderToWindow(RenderSystem::GetGameFrame());
		}

		// Deal with scene loading
		if (pPendingSceneLoad)
		{
			delete pCurrentScene;
			AssetDB::CollectGarbage();
			pCurrentScene = pPendingSceneLoad;
			if (pSceneCallBack)
				pSceneCallBack(*pCurrentScene);
			pPendingSceneLoad = nullptr;
		}

		// Framerate counter
		double realframeTime = double(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		if (realframeTime < targetFrameTime)
		{
			frameTime = targetFrameTime;
			unsigned int waitTime = int((targetFrameTime - realframeTime) * 1000.0);
			SDL_Delay(waitTime);
		}
		else
		{
			frameTime = realframeTime;
		}
		g_realFrameTime = realframeTime;
		g_observedFrameTime = double(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
	}

	delete pCurrentScene;
	AssetDB::CollectGarbage();

	// Shutdown everything
	RenderSystem::Destroy();
	AudioDevice::Destroy();
	GfxDevice::Destroy();
	AppWindow::Destroy();

	SDL_Quit();
}
