#pragma once

#include "GraphicsDevice.h"
#include "Scene.h"
#include "AssetDatabase.h"
#include "Mesh.h"

struct FrameContext;

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

	AssetHandle postProcessShader{ AssetHandle("Shaders/PostProcessing.hlsl") };
	AssetHandle bloomShader{ AssetHandle("Shaders/Bloom.hlsl") };

	// Graphics system resource handles
	RenderTargetHandle blurredFrame[2];
	ConstBufferHandle postProcessDataBuffer;
	ConstBufferHandle bloomDataBuffer;
	SamplerHandle fullScreenTextureSampler;
	
	Primitive fullScreenQuad{ Primitive::NewPlainQuad() };

	REFLECT()
};

namespace PostProcessingSystem
{
	void OnAddPostProcessing(Scene& scene, EntityID entity);
	void OnRemovePostProcessing(Scene& scene, EntityID entity);

	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
	void OnWindowResize(Scene& scene, float newWidth, float newHeight);
}
