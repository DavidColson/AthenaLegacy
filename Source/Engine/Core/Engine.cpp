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
#include "EASTL/sort.h"
#include "EASTL/bitvector.h"
#include "EASTL/fixed_list.h"

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

struct SortByArea
{
	bool operator()(const stbrp_rect& a, const stbrp_rect& b)
	{
		return (a.w * a.h) > (b.w * b.h);
	}
};

struct SortByHeight
{
	bool operator()(const stbrp_rect& a, const stbrp_rect& b)
	{
		return a.h > b.h;
	}
};

struct SortByWidth
{
	bool operator()(const stbrp_rect& a, const stbrp_rect& b)
	{
		return a.w> b.w;
	}
};

struct SortByPermimeter
{
	bool operator()(const stbrp_rect& a, const stbrp_rect& b)
	{
		return (2 * a.w + 2 * a.h) > (2 * b.w + 2 * b.h);
	}
};

void PackRectsNaiveRows(eastl::vector<stbrp_rect>& rects)
{
	// Sort by a heuristic
	eastl::sort(rects.begin(), rects.end(), SortByArea());

	USHORT xPos = 0;
	USHORT yPos = 0;

	USHORT largestHThisRow = 0;
	// Pack from left to right on a row

	int maxX = 0;
   	int maxY = 0;
   	int totalArea = 0;

	for (stbrp_rect& rect : rects)
	{
		if ((xPos + rect.w) > 700)
		{
			yPos += largestHThisRow;
			xPos = 0;
			largestHThisRow = 0;
		}

		// if ((yPos + rect.h) > 700)
		// 	break;

		rect.x = xPos;
		rect.y = yPos;

		xPos += rect.w;
		
		if (rect.h > largestHThisRow)
		{
			largestHThisRow = rect.h;
		}
		rect.was_packed = true;

        totalArea += rect.w * rect.h;
		int xExtent = rect.x + rect.w;
		int yExtent = rect.y + rect.h;
		if (xExtent > maxX)
			maxX = xExtent;
		if (yExtent > maxY)
			maxY = yExtent;
	}
	Log::Print(Log::EMsg, "maxY = %i maxX = %i Area Used %i rectsArea %i packing ratio %f", maxY, maxX, maxX * maxY, totalArea, (float)totalArea / float(maxX * maxY));
}

void PackRectsBLPixels(eastl::vector<stbrp_rect>& rects)
{
	// Sort by a heuristic
	eastl::sort(rects.begin(), rects.end(), SortByHeight());

	int maxX = 0;
   	int maxY = 0;
   	int totalArea = 0;

	// Maintain a grid of bits
	eastl::vector<eastl::vector<bool>> image;
	image.resize(700);
	for (int i=0; i< 700; i++)
	{
		image[i].resize(700, false);
	}

	for (stbrp_rect& rect : rects)
	{

		// Loop over X and Y
		bool done = false;
		for( int y = 0; y < 700 && !done; y++)
		{
			for( int x = 0; x < 700 && !done; x++)
			{
				// For every coordinate, check top left and bottom right
				if ((y + rect.h) >= 700 || (x + rect.w) >= 700)
					continue;

				if (!image[y][x] && !image[y + rect.h][x + rect.w])
				{
					// Corners of image are free
					// If valid, check all pixels inside that rect
					bool valid = true;
					for (int ix = x; ix < x + rect.w; ix += 5)
					{
						for (int iy = y; iy < y + rect.h; iy += 5)
						{
							if (image[iy][ix])
							{
								valid = false;
								break;
							}
						}
					}

					// If all good, we've found a location
					if (valid)
					{
						rect.x = (USHORT)x;
						rect.y = (USHORT)y;
						done = true;

						for (int ix = x; ix < x + rect.w; ix++)
						{
							for (int iy = y; iy < y + rect.h; iy++)
							{
								image[iy][ix] = true;
							}
						}

						rect.was_packed = true;
						totalArea += rect.w * rect.h;
						int xExtent = rect.x + rect.w;
						int yExtent = rect.y + rect.h;
						if (xExtent > maxX)
							maxX = xExtent;
						if (yExtent > maxY)
							maxY = yExtent;
					}
				}
			}
		}
	}
	Log::Print(Log::EMsg, "maxY = %i maxX = %i Area Used %i rectsArea %i packing ratio %f", maxY, maxX, maxX * maxY, totalArea, (float)totalArea / float(maxX * maxY));
}

struct Node
{
	int x, y = 0;
	int w, h = 0;
	bool filled = false;
};
constexpr int gridDim = 30;
typedef eastl::fixed_list<Node, gridDim> RowType;
typedef eastl::fixed_list<RowType, gridDim> GridType;

// xBegin and yBegin will be modified to give the iterators you need to split
bool CanBePlaced(RowType::iterator& xBegin, GridType::iterator& yBegin, int requiredHeight, int requiredWidth, int& outLeftOverHeight, int& outLeftOverWidth)
{
	int foundWidth = 0;
	int foundHeight = 0;

	GridType::iterator yIter = yBegin;
	RowType::iterator xIter = xBegin;
	int x = (int)eastl::distance((*yBegin).begin(), xBegin);

	while (foundHeight < requiredHeight) // Search through rows
	{
		xIter = (*yIter).begin();
		eastl::advance(xIter, x);
		foundWidth = 0;

		int rowHeight = 0;
		while (foundWidth < requiredWidth) // Search through columns
		{
			Node& node = *xIter;
			if (node.filled)
			{
				return false;
			}

			foundWidth += node.w;
			rowHeight = (*xIter).h;
			xIter++;
		}

		foundHeight += rowHeight;
		yIter++;
	}

	// The values of xIter-- and yIter-- are the iterators that need to be split
	xBegin = --xIter;
	yBegin = --yIter;

	outLeftOverWidth = (foundWidth - requiredWidth);
    outLeftOverHeight = (foundHeight - requiredHeight);

	return true;
}

// This just straight up doesn't work
void PackRectsGridSplitter(eastl::vector<stbrp_rect>& rects)
{
	// Sort by a heuristic
	eastl::sort(rects.begin(), rects.end(), SortByHeight());

	int maxX = 0;
   	int maxY = 0;
   	int totalArea = 0;

	GridType grid;

	// Initial node
	RowType newRow;
	newRow.push_back( Node{0, 0, 700, 700, false} );
	grid.insert(grid.begin(), newRow);

	for (stbrp_rect& rect : rects)
	{
		// Search through nodes looking for space
		bool done = false;
		for (GridType::iterator it = grid.begin(); it != grid.end() && !done; ++it)
		{
			RowType& row = *it;
			for (RowType::iterator rowIt = row.begin(); rowIt != row.end() && !done; ++rowIt)
			{
				Node& node = *rowIt;
				
				// While loop the X and Y axis until all the required height and width is found
				// If you hit an occupied cell, stop and fail

				int xStart = (int)eastl::distance((*it).begin(), rowIt);
				int yStart = (int)eastl::distance(grid.begin(), it);

				int leftOverWidth = 0;
				int leftOverHeight = 0;
				if (CanBePlaced(rowIt, it, rect.h, rect.w, leftOverHeight, leftOverWidth)) // This check must span multiple grid cells
				{
					// Found a suitable node
					done = true;
					rect.x = (USHORT)node.x;
					rect.y = (USHORT)node.y;

					// INSERT NEW ROW
					RowType rowCopy = *it;
					for (Node& newRowNode : rowCopy)
						newRowNode.h = newRowNode.h - leftOverHeight;
					for(Node& oldRowNode : row)
					{
						oldRowNode.y = oldRowNode.y + (oldRowNode.h - leftOverHeight);
						oldRowNode.h = leftOverHeight;
					}
					grid.insert(it, rowCopy);

					// INSERT NEW COLUMNS FOR EACH ROW
					int columnToBeSplit = (int)eastl::distance((*it).begin(), rowIt);
					int rowCounter = 0;
					for(RowType& aRow : grid)
					{
						RowType::iterator targetNodeIt = aRow.begin();
						eastl::advance(targetNodeIt, columnToBeSplit);
						
						// Update the width of the new and old versions
						// Copy the node at "rowIt"
						Node nodeCopy = *targetNodeIt;
						nodeCopy.w = nodeCopy.w - leftOverWidth;
						
						Node& originalNode = *targetNodeIt;
						originalNode.x = originalNode.x + (originalNode.w - leftOverWidth);
						originalNode.w = leftOverWidth;
						

						// Insert the new node at "rowIt"
						aRow.insert(targetNodeIt, nodeCopy);
						rowCounter++;
					}

					// LOOP OVER RELEVANT CELLS AND SET FILLED

					GridType::iterator startIt = grid.begin();
					eastl::advance(startIt, yStart);

					for (GridType::iterator filledIt = startIt; filledIt != it; ++filledIt)
					{
						RowType::iterator start = (*filledIt).begin();
						eastl::advance(start, xStart);

						RowType::iterator end = (*filledIt).begin();
						eastl::advance(end, columnToBeSplit + 1);

						for (RowType::iterator filledRowIt = start; filledRowIt != end; ++filledRowIt)
						{
							Node& filledNode = *filledRowIt;
							filledNode.filled = true;
						}
					}
				}
			}
		}

		if (!done) // failed
			continue;

		rect.was_packed = true;

        totalArea += rect.w * rect.h;
		int xExtent = rect.x + rect.w;
		int yExtent = rect.y + rect.h;
		if (xExtent > maxX)
			maxX = xExtent;
		if (yExtent > maxY)
			maxY = yExtent;
	}
	Log::Print(Log::EMsg, "maxY = %i maxX = %i Area Used %i rectsArea %i packing ratio %f", maxY, maxX, maxX * maxY, totalArea, (float)totalArea / float(maxX * maxY));
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

	Uint64 start = SDL_GetPerformanceCounter();

	//stbrp_pack_rects(&context, rects.data(), totalRects);
	//PackRectsNaiveRows(rects);
	PackRectsBLPixels(rects);
	//PackRectsGridSplitter(rects);

	double timeTaken = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
	Log::Print(Log::EMsg, "Time Taken: %.8f", timeTaken * 1000);

	STBRP_SORT(rects.data(), rects.size(), sizeof(rects[0]), rect_height_compare);


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

		bool open = true;
		ImGui::Begin("rect packer", &open);


		Vec2f topLeft = Vec2f(ImGui::GetWindowPos()) + Vec2f(10.0f, 50.0f);
		ImGui::GetWindowDrawList()->AddRect(topLeft, topLeft + Vec2f(700, 700), IM_COL32(255,255,255,255));
		
		int nPacked = 0;
		for	(int i = 0; i < totalRects; i++)
		{
			stbrp_rect& rect = rects.at(i);
			if (!rect.was_packed)
				continue;

			nPacked++;
			Vec2f rectPos = Vec2f((float)rect.x, (float)rect.y);
			Vec2f rectSize = Vec2f((float)rect.w, (float)rect.h);
			ImGui::GetWindowDrawList()->AddRectFilled(topLeft + rectPos, topLeft + rectPos + rectSize, rect.col);

			std::string label = StringFormat("%i", rect.id);
			const char* text_begin = label.c_str();
			const char* text_end = FindRenderedTextEnd(label.c_str(), NULL);
			ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 15.0f, topLeft + rectPos, IM_COL32(255,255,255,255), text_begin, text_end);
		}

		ImGui::Text("Packed rects %i", nPacked);

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
