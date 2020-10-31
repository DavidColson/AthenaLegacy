#pragma once

#include "Matrix.h"
#include "Vec2.h"
#include "GraphicsDevice.h"
#include "Scene.h"
#include "AssetDatabase.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Scene;
struct FrameContext;

// **********
// Components
// **********

struct CText
{
	eastl::string text;
	AssetHandle fontAsset;

	REFLECT()
};

struct Character
{
	Vec2i size{ Vec2i(0, 0) };
	Vec2i bearing{ Vec2i(0, 0) };
	Vec2f UV0{ Vec2f(0.f, 0.f) };
	Vec2f UV1{ Vec2f(1.f, 1.f) };
	int advance;
};

namespace FontSystem
{
	void Initialize();
	void Destroy();
	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);

	FT_Library GetFreeType();
}