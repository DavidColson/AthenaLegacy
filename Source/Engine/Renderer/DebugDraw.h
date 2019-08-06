#pragma once

#include "Maths/Vec2.h"
#include "Maths/Vec3.h"

namespace DebugDraw
{
	void Draw2DCircle(Vec2f pos, float radius, Vec3f color);
	void Draw2DLine(Vec2f start, Vec2f end, Vec3f color);

	namespace Detail
	{
		void Init();
		void DrawQueue();
	}
}
