#pragma once

#include <vector>
#include <array>

#include "Scene.h"
#include "GraphicsDevice/GraphicsDevice.h" 
#include "Renderer/RenderProxy.h"

class RenderFont;

namespace Renderer
{
	// Render System callbacks
	void OnGameStart_Deprecated(Scene& scene); // should eventually be unecessary, moved to other systems/components

	// Reactive Systems
	void OnPostProcessingAdded(Scene& scene, EntityID ent);

	// Systems
	void OnFrameStart(Scene& scene, float deltaTime);
	void OnFrame(Scene& scene, float deltaTime);

	// TODO: there should be a component removed reactive system that releases GfxDevice resources appropriately
};


// *******************
// Renderer Components
// *******************

struct CDrawable
{
	RenderProxy renderProxy;
	float lineThickness{ 1.5f };

	REFLECT()
};

struct CPostProcessing
{
	// Shader constants
	struct PostProcessShaderData
	{
		Vec2f resolution;
		float time{ 0.1f };
		float pad{ 0.0f };
	};

	struct BloomShaderData
	{
		Vec2f direction;
		Vec2f resolution;
	};

	// Graphics system resource handles
	RenderTargetHandle blurredFrame[2];
	ProgramHandle postProcessShaderProgram;
	ProgramHandle bloomShaderProgram;
	ConstBufferHandle postProcessDataBuffer;
	ConstBufferHandle bloomDataBuffer;

	REFLECT()
};