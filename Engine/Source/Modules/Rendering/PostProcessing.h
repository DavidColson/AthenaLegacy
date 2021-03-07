#pragma once

#include "GraphicsDevice.h"
#include "Scene.h"
#include "AssetDatabase.h"
#include "Mesh.h"

struct FrameContext;

namespace PostProcessing
{
	void Initialize();
	void Destroy();

	void OnFrame(FrameContext& ctx, float deltaTime);
	void OnWindowResize(float newWidth, float newHeight);
}
