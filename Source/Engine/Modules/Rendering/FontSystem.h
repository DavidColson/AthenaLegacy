#pragma once

#include "Matrix.h"
#include "Vec2.h"
#include "GraphicsDevice.h"
#include "Scene.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>

struct Scene;

struct Character
{
	Vec2i size{ Vec2i(0, 0) };
	Vec2i bearing{ Vec2i(0, 0) };
	Vec2f UV0{ Vec2f(0.f, 0.f) };
	Vec2f UV1{ Vec2f(1.f, 1.f) };
	int advance;
};

// **********
// Components
// **********

struct CText
{
	eastl::string text;

	REFLECT()
};

struct CFontSystemState
{
	ProgramHandle fontShaderProgram;
	ConstBufferHandle wvpBuffer;
	BlendStateHandle blendState;
	VertexBufferHandle vertexBuffer;
	IndexBufferHandle indexBuffer;
	TextureHandle fontTexture;
	SamplerHandle charTextureSampler;

	struct TransformData
	{
		Matrixf projection;
	};
	eastl::vector<Character> characters;
};

namespace FontSystem
{
	void OnAddFontSystemState(Scene& scene, EntityID entity);
	void OnRemoveFontSystemState(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}