#pragma once

#include "Matrix.h"
#include "Vec2.h"
#include "GraphicsDevice.h"
#include "Scene.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>

struct Scene;

// **********
// Components
// **********

struct CText
{
	eastl::string text;

	REFLECT()
};

namespace FontSystem
{
	void Initialize();
	void Destroy();
	void OnFrame(Scene& scene, float deltaTime);
}