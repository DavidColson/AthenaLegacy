#pragma once

#include "Maths/Maths.h"

namespace DebugDraw
{
	void Draw2DCircle(vec2 pos, float radius, vec3 color);
	void Draw2DLine(vec2 start, vec2 end, vec3 color);

	namespace Detail
	{
		void Init();
		void DrawQueue();
	}
}
