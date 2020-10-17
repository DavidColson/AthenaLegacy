#pragma once

#include "GraphicsDevice.h"
#include "Scene.h"
#include "AssetDatabase.h"

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

	AssetHandle postProcessShader{ AssetHandle("Engine/Resources/Shaders/PostProcessing.hlsl") };
	AssetHandle bloomShader{ AssetHandle("Engine/Resources/Shaders/Bloom.hlsl") };

	// Graphics system resource handles
	RenderTargetHandle blurredFrame[2];
	ConstBufferHandle postProcessDataBuffer;
	ConstBufferHandle bloomDataBuffer;
	VertexBufferHandle fullScreenQuad;
	SamplerHandle fullScreenTextureSampler;

	REFLECT()
};

namespace PostProcessingSystem
{
	void OnAddPostProcessing(Scene& scene, EntityID entity);
	void OnRemovePostProcessing(Scene& scene, EntityID entity);

	void OnFrame(Scene& scene, float deltaTime);
	void OnWindowResize(Scene& scene, float newWidth, float newHeight);
}
