#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <comdef.h>
#include <vector>
#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "Rendering/ParticlesSystem.h"
#include "Rendering/FontSystem.h"
#include "Rendering/PostProcessingSystem.h"
#include "Rendering/DebugDraw.h"
#include "Rendering/ShapesSystem.h"
#include "AudioDevice.h"
#include "Scene.h"
#include "Input/Input.h"
#include "Editor/Editor.h"
#include "Log.h"
#include "Profiler.h"
#include "Memory.h"

#include "EASTL/vector.h"
#include "EASTL/fixed_vector.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

namespace
{
	// Frame stats
	double g_observedFrameTime;
	double g_realFrameTime;

	bool g_gameRunning{ true };

	EntityID g_engineSingleton{ INVALID_ENTITY };

	Scene* pCurrentScene{ nullptr };
	Scene* pPendingSceneLoad{ nullptr };
	SDL_Window* g_pWindow{ nullptr };
}

char* readFile(const char* filename)
{
	SDL_RWops* rw = SDL_RWFromFile(filename, "r+");
	if (rw == nullptr)
	{
		Log::Print(Log::EErr, "%s failed to load", filename);
		return nullptr;
	}

	size_t fileSize = SDL_RWsize(rw);

	char* buffer = new char[fileSize];
	SDL_RWread(rw, buffer, sizeof(char) * fileSize, 1);
	SDL_RWclose(rw);
	buffer[fileSize] = '\0';
	return buffer;
}

void ImGuiPreUpdate(Scene& /* scene */, float /* deltaTime */)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void ImGuiRender(Scene& /* scene */, float /* deltaTime */)
{
	GFX_SCOPED_EVENT("Drawing imgui");
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnAdd, ParticlesSystem::OnAddEmitter);
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnRemove, ParticlesSystem::OnRemoveEmitter);

	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnAdd, PostProcessingSystem::OnAddPostProcessing);
	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnRemove, PostProcessingSystem::OnRemovePostProcessing);

	scene.RegisterReactiveSystem<CDebugDrawingState>(Reaction::OnAdd, DebugDraw::OnDebugDrawStateAdded);
	scene.RegisterReactiveSystem<CDebugDrawingState>(Reaction::OnRemove, DebugDraw::OnDebugDrawStateRemoved);

	scene.RegisterReactiveSystem<CShapesSystemState>(Reaction::OnAdd, Shapes::OnShapesSystemStateAdded);
	scene.RegisterReactiveSystem<CShapesSystemState>(Reaction::OnRemove, Shapes::OnShapesSystemStateRemoved);

	scene.RegisterReactiveSystem<CFontSystemState>(Reaction::OnAdd, FontSystem::OnAddFontSystemState);
	scene.RegisterReactiveSystem<CFontSystemState>(Reaction::OnRemove, FontSystem::OnRemoveFontSystemState);

	scene.RegisterSystem(SystemPhase::PreUpdate, ImGuiPreUpdate);
	scene.RegisterSystem(SystemPhase::Update, Input::OnFrame);
	scene.RegisterSystem(SystemPhase::Update, Editor::OnFrame);
	// scene.RegisterSystem(SystemPhase::Render, Shapes::OnFrame);
	// scene.RegisterSystem(SystemPhase::Render, ParticlesSystem::OnFrame);
	// scene.RegisterSystem(SystemPhase::Render, FontSystem::OnFrame);
	// scene.RegisterSystem(SystemPhase::Render, DebugDraw::OnFrame);
	// scene.RegisterSystem(SystemPhase::Render, PostProcessingSystem::OnFrame);
	scene.RegisterSystem(SystemPhase::Render, ImGuiRender);

	scene.NewEntity("Engine Singletons");
	scene.Assign<CDebugDrawingState>(ENGINE_SINGLETON);
	scene.Assign<CShapesSystemState>(ENGINE_SINGLETON);
	scene.Assign<CFontSystemState>(ENGINE_SINGLETON);
}

void Engine::SetActiveScene(Scene* pScene)
{
	pPendingSceneLoad = pScene;
}

void Engine::Initialize()
{
	// Startup flow
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	float width = 1800.0f;
	float height = 1000.0f;

	g_pWindow = SDL_CreateWindow(
		"Asteroids",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		0
	);


	eastl::fixed_vector<int, 100> vec;

	vec.push_back(4);
	vec.push_back(2);
	vec.push_back(7);

	for (int i = 0; i < 100000; i++)
	{
		vec.push_back(2);
	}

	Log::Print(Log::EMsg, "Engine starting up");
	Log::Print(Log::EMsg, "Window size W: %.1f H: %.1f", width, height);

	GfxDevice::Initialize(g_pWindow, width, height);
	AudioDevice::Initialize();
	Input::CreateInputState();
}

const char* FindRenderedTextEnd(const char* text, const char* text_end)
{
    const char* text_display_end = text;
    if (!text_end)
        text_end = (const char*)-1;

    while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
        text_display_end++;
    return text_display_end;
}

void Engine::Run(Scene *pScene)
{	
	pCurrentScene = pScene;
	
	// Start packing
	int totalRects = 25;
    eastl::vector<stbrp_node> nodes;
    nodes.resize(totalRects);
    stbrp_context context;
    stbrp_init_target(&context, 700, 700, nodes.data(), totalRects);

	eastl::vector<stbrp_rect> rects;
	rects.resize(totalRects);
	for(int i = 0; i < totalRects; i++)
	{	
		stbrp_rect& rect = rects.at(i);
		rect.w = (uint16_t)float(30 + rand() % 180);
		rect.h = (uint16_t)float(30 + rand() % 180);
		rect.id = i + 1;
		rect.col = IM_COL32(rand() % 256, rand() % 256, rand() % 256,255);
	}

	// Uint64 start = SDL_GetPerformanceCounter();

	// stbrp_pack_rects(&context, rects.data(), totalRects);

	// double timeTaken = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
	// Log::Print(Log::EMsg, "Time Taken: %.8f", timeTaken * 1000);

	STBRP_SORT(rects.data(), rects.size(), sizeof(rects[0]), rect_height_compare);

	int nPacked = 0;

	float packTimer = 5.0f;

	// Game update loop
	double frameTime = 0.016f;
	double targetFrameTime = 0.016f;
	while (g_gameRunning)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		pCurrentScene->SimulateScene((float)frameTime);

		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

		// Make sure we sort the rectangles before packing them!
		if ((Input::GetKeyDown(SDL_SCANCODE_E) || packTimer < 0.0f) && nPacked < totalRects)
		{
			stbrp_rect& rect = rects.at(nPacked);
			stbrp_pack_rects(&context, &rect, 1);
			nPacked++;
			packTimer = 0.5f;
		}
		//packTimer -= (float)frameTime;

		bool open = true;
		ImGui::Begin("rect packer", &open);

		ImGui::Text("Packed rects %i", nPacked);

		Vec2f topLeft = Vec2f(ImGui::GetWindowPos()) + Vec2f(10.0f, 50.0f);
		ImGui::GetWindowDrawList()->AddRect(topLeft, topLeft + Vec2f(700, 700), IM_COL32(255,255,255,255));
		
		for	(int i = 0; i < nPacked; i++)
		{
			stbrp_rect& rect = rects.at(i);
			Vec2f rectPos = Vec2f((float)rect.x, (float)rect.y);
			Vec2f rectSize = Vec2f((float)rect.w, (float)rect.h);
			ImGui::GetWindowDrawList()->AddRectFilled(topLeft + rectPos, topLeft + rectPos + rectSize, rect.col);

			std::string label = StringFormat("%i", rect.id);
			const char* text_begin = label.c_str();
			const char* text_end = FindRenderedTextEnd(label.c_str(), NULL);
			ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 25.0f, topLeft + rectPos, IM_COL32(255,255,255,255), text_begin, text_end);
		}

		topLeft = Vec2f(ImGui::GetWindowPos()) + Vec2f(10.0f, 780.0f);
		stbrp_node* currNode = context.active_head;
		int count = 0;
		while (currNode != nullptr)
		{
			if (currNode->id < 0 || currNode->id > totalRects) 
			{
				currNode = currNode->next;
				continue;
			}
			ImU32 col =  currNode->col;
			ImGui::GetWindowDrawList()->AddRectFilled(topLeft + Vec2f(count * 30.0f, 0.0f), topLeft + Vec2f(count * 30.0f, 0.0f) + Vec2f(30.0f, 30.0f), col);

			std::string label = StringFormat("%i", currNode->id);
			const char* text_begin = label.c_str();
			const char* text_end = FindRenderedTextEnd(label.c_str(), NULL);
			ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 15.0f, topLeft + Vec2f(count * 30.0f, 0.0f), IM_COL32(255,255,255,255), text_begin, text_end);
			count++;
			currNode = currNode->next;
		}
		
		
		ImGui::End();

		pCurrentScene->RenderScene((float)frameTime);

		GfxDevice::PresentBackBuffer();
		GfxDevice::ClearRenderState();
		GfxDevice::PrintQueuedDebugMessages();




		if (pPendingSceneLoad)
		{
			delete pCurrentScene;
			pCurrentScene = pPendingSceneLoad;
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

	// Shutdown everything
	AudioDevice::Destroy();
	GfxDevice::Destroy();

	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}
