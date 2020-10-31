#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Scene.h"

struct FrameContext;

namespace DebugDraw
{	
	void Draw2DCircle(Scene& scene, Vec2f pos, float radius, Vec3f color);
	void Draw2DLine(Scene& scene, Vec2f start, Vec2f end, Vec3f color);

	void Initialize();
	void Destroy();
	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
	void OnFrameEnd(Scene& scene, float deltaTime);
}