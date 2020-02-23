#pragma once

#include "GraphicsDevice.h"
#include "Scene.h"

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
	VertexBufferHandle fullScreenQuad;
	SamplerHandle fullScreenTextureSampler;

	REFLECT()
};

namespace PostProcessingSystem
{
	void OnPostProcessingAdded(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}