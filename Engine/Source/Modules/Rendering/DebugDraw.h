#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Scene.h"

struct FrameContext;

namespace DebugDraw
{	
	void Draw2DCircle(Vec2f pos, float radius, Vec4f color);
	void Draw2DLine(Vec2f start, Vec2f end, Vec4f color);
	
	void DrawLine(Vec3f start, Vec3f end, Vec4f color);

	void Initialize();
	void Destroy();
	void OnFrame(FrameContext& ctx, float deltaTime);
	void OnFrameEnd(float deltaTime);
}