#pragma once

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Scene.h"

typedef eastl::fixed_vector<Vec2f, 50> VertsVector;

namespace Shapes
{	
	void DrawPolyLine(Scene& scene, const VertsVector& verts, float thickness, Vec3f color, bool connected = true);

	void Initialize();
	void Destroy();
	void OnFrame(Scene& scene, float deltaTime);
}