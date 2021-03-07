#include "Rendering/PostProcessing.h"

#include "Profiler.h"
#include "Mesh.h"
#include "Shader.h"
#include <SDL_timer.h>
#include "GameRenderer.h"

namespace
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
	
	Primitive fullScreenQuad;
};

// ***********************************************************************

void PostProcessing::Initialize()
{
    fullScreenQuad = Primitive::NewPlainQuad();
    
	for (int i = 0; i < 2; ++i)
	{
		blurredFrame[i] = GfxDevice::CreateRenderTarget(GameRenderer::GetWidth() / 2.0f, GameRenderer::GetHeight() / 2.0f, 1, eastl::string().sprintf("Blurred frame %i", i));
	}

	// Create constant data buffers
	postProcessDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(PostProcessShaderData), "Post process shader data");
	bloomDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(BloomShaderData), "Bloom shader data");
    fullScreenTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Wrap, "Fullscreen texture");
}

// ***********************************************************************

void PostProcessing::Destroy()
{
	GfxDevice::FreeConstBuffer(postProcessDataBuffer);
	GfxDevice::FreeConstBuffer(bloomDataBuffer);
	GfxDevice::FreeSampler(fullScreenTextureSampler);
	GfxDevice::FreeRenderTarget(blurredFrame[0]);
	GfxDevice::FreeRenderTarget(blurredFrame[1]);
}

// ***********************************************************************

void PostProcessing::OnFrame(FrameContext& ctx, float deltaTime)
{
    PROFILE();
    GFX_SCOPED_EVENT("Doing post processing");	
    TextureHandle preProcessedFrame = GfxDevice::MakeResolvedTexture(ctx.backBuffer);

    GfxDevice::BindRenderTarget(blurredFrame[0]);
    GfxDevice::ClearRenderTarget(blurredFrame[0], { 0.0f, 0.f, 0.f, 1.0f }, true, true);
    GfxDevice::SetViewport(0.f, 0.f, ctx.screenDimensions.x / 2.0f, ctx.screenDimensions.y / 2.0f);
    GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

    // Bind bloom shader data
    Shader* pShader = AssetDB::GetAsset<Shader>(bloomShader);
    GfxDevice::BindProgram(pShader->program);
    GfxDevice::BindVertexBuffers(0, 1, &fullScreenQuad.bufferHandle_vertices);
    GfxDevice::BindVertexBuffers(1, 1, &fullScreenQuad.bufferHandle_uv0);
    GfxDevice::BindSampler(fullScreenTextureSampler, ShaderType::Pixel, 0);

    BloomShaderData bloomData;
    bloomData.resolution = Vec2f(900, 500);
    
    // Iteratively calculate bloom
    int blurIterations = 8;
    for (int i = 0; i < blurIterations; ++i)
    {
        // Bind data
        float radius = float(blurIterations - i - 1) * 0.04f;
        bloomData.direction = i % 2 == 0 ? Vec2f(radius, 0.0f) : Vec2f(0.0f, radius);
        GfxDevice::BindConstantBuffer(bloomDataBuffer, &bloomData, ShaderType::Pixel, 0);

        if (i == 0)
        {
            // First iteration, bind the plain, preprocessed frame
            GfxDevice::BindTexture(preProcessedFrame, ShaderType::Pixel, 0);
        }
        else
        {
            GfxDevice::UnbindRenderTarget(blurredFrame[(i + 1) % 2]);
            TextureHandle tex = GfxDevice::GetTexture(blurredFrame[(i + 1) % 2]);
            GfxDevice::BindTexture(tex, ShaderType::Pixel, 0);
            GfxDevice::BindRenderTarget(blurredFrame[i % 2]);
            GfxDevice::ClearRenderTarget(blurredFrame[i % 2], { 0.0f, 0.f, 0.f, 1.0f }, true, true);
        }

        GfxDevice::Draw(4, 0);
    }

    // Now we'll actually render onto the backbuffer and do our final post process stage
    GameRenderer::SetBackBufferActive();
    GameRenderer::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f }, true, true);
    GfxDevice::SetViewport(0, 0, ctx.screenDimensions.x, ctx.screenDimensions.y);

    Shader* pPPShader = AssetDB::GetAsset<Shader>(postProcessShader);
    GfxDevice::BindProgram(pPPShader->program);

    GfxDevice::BindTexture(preProcessedFrame, ShaderType::Pixel, 0);
    TextureHandle blurFrameTex = GfxDevice::GetTexture(blurredFrame[1]);
    GfxDevice::BindTexture(blurFrameTex, ShaderType::Pixel, 1);

    GfxDevice::BindSampler(fullScreenTextureSampler, ShaderType::Pixel, 0);
    
    PostProcessShaderData ppData;
    ppData.resolution = Vec2f(ctx.screenDimensions.x, ctx.screenDimensions.y);
    ppData.time = float(SDL_GetTicks()) / 1000.0f;
    GfxDevice::BindConstantBuffer(postProcessDataBuffer, &ppData, ShaderType::Pixel, 0);

    // Draw post processed frame
    GfxDevice::Draw(4, 0);

    GfxDevice::FreeTexture(preProcessedFrame);
}

// ***********************************************************************

void PostProcessing::OnWindowResize(float newWidth, float newHeight)
{
    for (int i = 0; i < 2; ++i)
    {
        GfxDevice::FreeRenderTarget(blurredFrame[i]);
        blurredFrame[i] = GfxDevice::CreateRenderTarget(GameRenderer::GetWidth() / 2.0f, GameRenderer::GetHeight() / 2.0f, 1, eastl::string().sprintf("Blurred frame %i", i));
    }
}