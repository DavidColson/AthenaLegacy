#pragma once

#include "Matrix.h"
#include "Vec2.h"
#include "Renderer/Renderer.h"
#include "GraphicsDevice.h"
#include "Scene.h"

#include <string>
#include <vector>

struct Scene;

struct Character
{
	TextureHandle charTexture;
	Vec2i size{ Vec2i(0, 0) };
	Vec2i bearing{ Vec2i(0, 0) };
	int advance;
};

// **********
// Components
// **********

struct CText
{
	std::string text;

	REFLECT()
};

struct CFontSystemState
{
	VertexBufferHandle quadBuffer;
	SamplerHandle charTextureSampler;
	ProgramHandle fontShaderProgram;
	ConstBufferHandle wvpBuffer;

	struct TransformData
	{
		Matrixf wvp;
	};
	std::vector<Character> characters;
};

namespace FontSystem
{
	void OnAddFontSystemState(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}