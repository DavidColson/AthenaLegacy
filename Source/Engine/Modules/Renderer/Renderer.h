#pragma once

#include <vector>
#include <array>

#include "Matrix.h"
#include "Scene.h"
#include "GraphicsDevice.h" 

class RenderFont;

namespace Renderer
{
	// Systems
	void OnFrameStart(Scene& scene, float deltaTime);
	void OnFrame(Scene& scene, float deltaTime);

	// TODO: there should be a component removed reactive system that releases GfxDevice resources appropriately
};