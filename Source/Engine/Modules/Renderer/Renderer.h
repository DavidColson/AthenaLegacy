#pragma once

#include <vector>
#include <array>

#include "Matrix.h"
#include "Scene.h"
#include "GraphicsDevice.h" 

class RenderFont;

namespace Renderer
{
	// Render System callbacks
	void OnGameStart_Deprecated(Scene& scene); // should eventually be unecessary, moved to other systems/components

	// Systems
	void OnFrameStart(Scene& scene, float deltaTime);
	void OnFrame(Scene& scene, float deltaTime);

	// TODO: there should be a component removed reactive system that releases GfxDevice resources appropriately
};