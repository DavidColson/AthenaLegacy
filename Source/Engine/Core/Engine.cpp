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
	eastl::sort(rects.begin(), rects.end(), SortByHeight());

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

struct DynamicGrid
{
	void Init(int width, int height)
	{
		data.resize(gridSize * gridSize, false);

		rows.reserve(gridSize);
		rows.push_back({width, 0});

		columns.reserve(gridSize);
		columns.push_back({height, 0});
	}

	void InsertRow(int atY, int oldRowHeight)
	{
		// Copy the row atY to the end of data (add a new row)
		int rowIndex = rows[atY].index;
		for (int i = 0; i < columns.size(); i++)
		{
			data[GetDataLocation(i, (int)rows.size())] = data[GetDataLocation(i, rowIndex)];
		}
		
		Dimension old = rows[atY];

		// Insert a new element into "rows"
		// the index of the new element to the end of the data array
		rows.insert(rows.begin() + atY, Dimension{old.size - oldRowHeight, (int)rows.size()});

		// Set the size of the old and new rows to the appropriate heights
		rows[atY+1].size = oldRowHeight;
		
	}

	void InsertColumn(int atX, int oldRowWidth)
	{
		// Copy the column atX to the end of data (add a new column)
		int columnIndex = columns[atX].index;
		for (int i = 0; i < rows.size(); i++)
		{
			data[GetDataLocation((int)columns.size(), i)] = data[GetDataLocation(columnIndex, i)];
		}

		Dimension old = columns[atX];

		// Insert a new element into "columns"
		// the index of the new element to the end of the data array
		columns.insert(columns.begin() + atX, Dimension{old.size - oldRowWidth, (int)columns.size()});

		// Set the size of the old and new columns to the appropriate heights
		columns[atX+1].size = oldRowWidth;
		
	}

	bool Get(int x, int y)
	{
		int rowIndex = rows[y].index;
		int columnIndex = columns[x].index;
		return data[GetDataLocation(columnIndex, rowIndex)];
	}

	void Set(int x, int y, bool val)
	{
		int rowIndex = rows[y].index;
		int columnIndex = columns[x].index;
		data[GetDataLocation(columnIndex, rowIndex)] = val;
	}

	inline int GetDataLocation(int x, int y)
	{
		return gridSize * y + x;
	}

	inline int GetRowHeight(int y)
	{
		return rows[y].size;
	}

	inline int GetColumnWidth(int x)
	{
		return columns[x].size;
	}

	void PrintGrid()
	{
		Log::PrintNoNewLine("Current Grid State\n");
		for (int y = 0; y < rows.size(); y++)
		{
			Log::PrintNoNewLine("{ ");
			for (int x = 0; x < columns.size(); x++)
			{
				Log::PrintNoNewLine("%i ", int(Get(x, y)));
			}
			Log::PrintNoNewLine("}\n");
		}
	}

	eastl::vector<bool> data;
	int gridSize = 400;

	struct Dimension
	{
		int size;
		int index;
	};
	eastl::vector<Dimension> rows;
	eastl::vector<Dimension> columns;
};

// xBegin and yBegin will be modified to give the iterators you need to split
bool CanBePlaced(DynamicGrid& grid, Vec2i desiredNode, Vec2i desiredRectSize, Vec2i& outRequiredNodes, Vec2i& outRemainingSize)
{
	int foundWidth = 0;
	int foundHeight = 0;

	// For tracking which cells we're checking in the grid
	// Remember we have to check multiple cells as the rect could span a few cells
	int trialX = desiredNode.x;
	int trialY = desiredNode.y;
	while (foundHeight < desiredRectSize.y)
	{
		trialX = desiredNode.x;
		foundWidth = 0;

		while (foundWidth < desiredRectSize.x)
		{
			if (trialX >= grid.columns.size() || trialY >= grid.rows.size())
			{
				return false; // ran out of space
			}
			
			if (grid.Get(trialX, trialY))
			{
				return false;
			}

			foundWidth += grid.GetColumnWidth(trialX);
			trialX++;
		}

		foundHeight += grid.GetRowHeight(trialY);
		trialY++;
	}

	// Visited all cells that we'll need to place the rectangle,
	// and none were occupied. So the space is available here.
	outRequiredNodes = Vec2i(trialX - desiredNode.x, trialY - desiredNode.y);
	outRemainingSize = Vec2i(foundWidth - desiredRectSize.x, foundHeight - desiredRectSize.y);

	return true;
}

DynamicGrid PackRectsGridSplitter(eastl::vector<stbrp_rect>& rects)
{
	// Sort by a heuristic
	eastl::sort(rects.begin(), rects.end(), SortByHeight());

	for(int i = 0; i < rects.size(); i++)
	{	
		stbrp_rect& rect = rects.at(i);
		rect.id = i;
	}

	int maxX = 0;
   	int maxY = 0;
   	int totalArea = 0;

	DynamicGrid grid;
	grid.Init(700, 700);
	
	for (stbrp_rect& rect : rects)
	{
		// Search through nodes looking for space
		bool done = false;
		int yPos = 0;
		for (int y = 0; y < grid.rows.size() && !done; y++)
		{
			int xPos = 0;
			for (int x = 0; x < grid.columns.size() && !done; x++)
			{
				Vec2i leftOverSize;
				Vec2i requiredNodes;
				if (CanBePlaced(grid, Vec2i(x, y), Vec2i(rect.w, rect.h), requiredNodes, leftOverSize))
				{
					done = true;
					rect.x = (USHORT)xPos;
					rect.y = (USHORT)yPos;

					int xFarRightColumn = x + requiredNodes.x - 1;
					grid.InsertColumn(xFarRightColumn, leftOverSize.x);

					int yFarBottomRow = y + requiredNodes.y - 1;
					grid.InsertRow(yFarBottomRow, leftOverSize.y);

					for (int i = x + requiredNodes.x - 1; i >= x; i--)
					{
						for (int j = y + requiredNodes.y - 1; j >= y; j--)
						{
							grid.Set(i, j, true);
						}
					}
				}
				xPos += grid.GetColumnWidth(x);
			}
			yPos += grid.GetRowHeight(y);
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
	return grid;
}

struct Node
{
	int x, y;
	int w, h;
};

eastl::vector<Node> PackRectsBinaryTree(eastl::vector<stbrp_rect>& rects)
{
	// Sort by a heuristic
	eastl::sort(rects.begin(), rects.end(), SortByArea());

	for(int i = 0; i < rects.size(); i++)
	{	
		stbrp_rect& rect = rects.at(i);
		rect.id = i;
	}

	int maxX = 0;
   	int maxY = 0;
   	int totalArea = 0;
	
	eastl::vector<Node> leaves;

	leaves.push_back({0, 0, 700, 700});

	for (stbrp_rect& rect : rects)
	{
		// BINARY TREES
		bool done = false;
		for (int i = (int)leaves.size() - 1; i >= 0 && !done; --i)
		{
			Node& node = leaves[i];

			if (node.w > rect.w && node.h > rect.h)
			{
				// Found a suitable node
				rect.x = node.x;
				rect.y = node.y;

				// Split it
				int remainingWidth = node.w - rect.w;
				int remainingHeight = node.h - rect.h;

				// The lesser split here will be the top right
				Node newSmallerNode;
				Node newLargerNode;
				if (remainingHeight > remainingWidth)
				{
					newSmallerNode.x = node.x + rect.w;
					newSmallerNode.y = node.y;
					newSmallerNode.w = remainingWidth;
					newSmallerNode.h = rect.h;

					newLargerNode.x = node.x;
					newLargerNode.y = node.y + rect.h;
					newLargerNode.w = node.w;
					newLargerNode.h = remainingHeight;
				}
				// The lesser split here will be the bottom left
				else
				{
					newSmallerNode.x = node.x;
					newSmallerNode.y = node.y + rect.h;
					newSmallerNode.w = rect.w;
					newSmallerNode.h = remainingHeight;

					newLargerNode.x = node.x + rect.w;
					newLargerNode.y = node.y;
					newLargerNode.w = remainingWidth;
					newLargerNode.h = node.h;
				}

				// Removing the node we're using up
				leaves[i] = leaves.back();
				leaves.pop_back();

				leaves.push_back(newLargerNode);
				leaves.push_back(newSmallerNode);
				
				done = true;
			}
		}

		if (!done)
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
	return leaves;
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
	//PackRectsBLPixels(rects);
	//DynamicGrid grid = PackRectsGridSplitter(rects);
	eastl::vector<Node> leaves = PackRectsBinaryTree(rects);

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
		for	(int i = 0; i < totalRects && true; i++)
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

		// Debug code for seeing binary tree splits

		for (int i = (int)leaves.size() - 1; i >= 0; --i)
		{
			Node& node = leaves[i];

			// horizontal line
			ImGui::GetWindowDrawList()->AddLine(
				topLeft + Vec2f(float(node.x), float(node.y)),
				topLeft + Vec2f(float(node.x + node.w), float(node.y)),
				IM_COL32(255,255,255,255));

			// vertical line
			ImGui::GetWindowDrawList()->AddLine(
				topLeft + Vec2f(float(node.x), float(node.y)),
				topLeft + Vec2f(float(node.x), float(node.y + node.h)),
				IM_COL32(255,255,255,255));

			// std::string label = StringFormat("%i", i);
			// const char* text_begin = label.c_str();
			// const char* text_end = FindRenderedTextEnd(label.c_str(), NULL);
			// ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 15.0f, topLeft + Vec2f(float(node.x), float(node.y)), IM_COL32(255,255,255,255), text_begin, text_end);
		}


		// Debug code for seeing grids

		// Search through nodes looking for space
		// int xPos = 0;
		// for (int x = 0; x < grid.columns.size(); x++)
		// {
		// 	xPos += grid.columns[x].size;
		// 	ImGui::GetWindowDrawList()->AddLine(
		// 		topLeft + Vec2f(float(xPos),0),
		// 		topLeft + Vec2f(float(xPos), float(700)),
		// 		IM_COL32(255,255,255,255));
		// }
		// int yPos = 0;
		// for (int y = 0; y < grid.rows.size(); y++)
		// {

		// 	int xPos = 0;
		// 	for (int x = 0; x < grid.columns.size(); x++)
		// 	{
		// 		if (grid.Get(x, y))
		// 		{
		// 			Vec2f rectPos = Vec2f((float)xPos, (float)yPos);
		//  			Vec2f rectSize = Vec2f((float)grid.columns[x].size, (float)grid.rows[y].size);
		//  			ImGui::GetWindowDrawList()->AddRectFilled(topLeft + rectPos, topLeft + rectPos + rectSize, IM_COL32(0,0,0,200));
		// 		}
		// 		xPos += grid.columns[x].size;
		// 	}
		// 	yPos += grid.rows[y].size;
		// 	ImGui::GetWindowDrawList()->AddLine(
		// 		topLeft + Vec2f(0, float(yPos)),
		// 		topLeft + Vec2f(float(700), float(yPos)),
		// 		IM_COL32(255,255,255,255));
		// }


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
