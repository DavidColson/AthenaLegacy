#include "Engine.h"

#include <SDL.h>

#include "AppWindow.h"
#include "GraphicsDevice.h"
#include "Rendering/GameRenderer.h"
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
#include "FileSystem.h"

REFLECT_BEGIN(EngineConfig)
REFLECT_MEMBER(windowName)
REFLECT_MEMBER(windowResolution)
REFLECT_MEMBER(bootInEditor)
REFLECT_MEMBER(hotReloadingAssetsEnabled)
REFLECT_MEMBER(gameResourcesPath)
REFLECT_MEMBER(engineResourcesPath)
REFLECT_END()

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	bool g_gameRunning{ true };
	EngineConfig config;
	eastl::string configFileName;

	// Scene management
	void (*pSceneCallBack)(Scene& scene);
	Scene* pCurrentScene{ nullptr };
	Scene* pPendingSceneLoad{ nullptr };
}

// ***********************************************************************

void Engine::GetFrameRates(double& outReal, double& outLimited)
{
	outReal = g_realFrameTime;
	outLimited = g_observedFrameTime;
}

// ***********************************************************************

void Engine::StartShutdown()
{
	g_gameRunning = false;
}

// ***********************************************************************

void Engine::NewSceneCreated(Scene& scene)
{
	GameRenderer::OnSceneCreate(scene);

	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, TransformHeirarchy);

	// @Improvement consider allowing engine config files to change the update order of systems
}

// ***********************************************************************

void Engine::SetActiveScene(Scene* pScene)
{
	pPendingSceneLoad = pScene;
}

// ***********************************************************************

void Engine::SetSceneCreateCallback(void (*pCallBackFunc)(Scene&))
{
	pSceneCallBack = pCallBackFunc;
}

// ***********************************************************************

const EngineConfig& Engine::GetConfig()
{
	return config;
}

// ***********************************************************************

void Engine::SaveConfig(const eastl::string& fileName)
{
	JsonValue configJson = EngineConfig::typeData.ToJson(config);
	FileSys::WriteWholeFile(fileName, SerializeJsonValue(configJson));
	configFileName = fileName;
}

// ***********************************************************************

void Engine::Initialize(const char* _configFileName)
{
	configFileName = _configFileName;

	if (FileSys::Exists(configFileName))
	{
		Log::Info("Config file loaded from '%s'", _configFileName);

		JsonValue configJson = ParseJsonFile(FileSys::ReadWholeFile(configFileName));
		config = EngineConfig::typeData.FromJson(configJson).GetValue<EngineConfig>();
	}
	else
	{
		Log::Info("No config file found, making new one at '%s'", _configFileName);

		Path pathToFile = Path(configFileName).ParentPath();
		FileSys::NewDirectories(pathToFile);
		JsonValue configJson = EngineConfig::typeData.ToJson(config);
		FileSys::WriteWholeFile(configFileName, SerializeJsonValue(configJson));
	}

	Initialize(config);
}

// ***********************************************************************

void Engine::Initialize(const EngineConfig& _config)
{
	config = _config;
	
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	AppWindow::Create(config.windowResolution.x, config.windowResolution.y, config.windowName);

	Log::SetLogLevel(Log::EDebug);

	GameRenderer::Initialize(config.windowResolution.x, config.windowResolution.y);
	AudioDevice::Initialize();
	Input::CreateInputState();	
	Editor::Initialize(config.bootInEditor);
}

// ***********************************************************************

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

		if (config.hotReloadingAssetsEnabled)
			AssetDB::UpdateHotReloading();

		// Deal with events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			Editor::ProcessEvent(*pCurrentScene, &event);
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					if (event.window.windowID == SDL_GetWindowID(AppWindow::GetSDLWindow()))
					{
						AppWindow::Resize((float)event.window.data1, (float)event.window.data2);

						if (Editor::IsInEditor())
							Editor::ResizeEditorFrame((float)event.window.data1, (float)event.window.data2);
						else
							GameRenderer::ResizeGameFrame(*pCurrentScene, (float)event.window.data1, (float)event.window.data2);
					}
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				Engine::StartShutdown();
				break;
			}
		}

		// Preparing editor code now allows game code to define it's own editors 
		Editor::PreUpdate();

		// Simulate current game scene
		pCurrentScene->SimulateScene((float)frameTime);
		Profiler::ClearFrameData();

		// Render the game
		TextureHandle gameFrame = GameRenderer::DrawFrame(*pCurrentScene, (float)frameTime);

		// Render the editor
		TextureHandle editorFrame = Editor::DrawFrame(*pCurrentScene, (float)frameTime);
		
		if (Editor::IsInEditor())
			AppWindow::RenderToWindow(editorFrame);
		else
			AppWindow::RenderToWindow(gameFrame);

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
	GameRenderer::Destroy();
	AudioDevice::Destroy();
	Editor::Destroy();
	GfxDevice::Destroy();
	AppWindow::Destroy();

	SDL_Quit();
}
