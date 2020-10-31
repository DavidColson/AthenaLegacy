#include "Rendering/PostProcessingSystem.h"

#include "Profiler.h"
#include "Mesh.h"
#include "Shader.h"
#include <SDL_timer.h>
#include "GameRenderer.h"

REFLECT_COMPONENT_BEGIN(CPostProcessing)
REFLECT_END()

// ***********************************************************************

void PostProcessingSystem::OnAddPostProcessing(Scene& scene, EntityID entity)
{
    CPostProcessing& pp = *(scene.Get<CPostProcessing>(entity));
	for (int i = 0; i < 2; ++i)
	{
		pp.blurredFrame[i] = GfxDevice::CreateRenderTarget(GameRenderer::GetWidth() / 2.0f, GameRenderer::GetHeight() / 2.0f, 1, eastl::string().sprintf("Blurred frame %i", i));
	}

	// Create constant data buffers
	pp.postProcessDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::PostProcessShaderData), "Post process shader data");
	pp.bloomDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::BloomShaderData), "Bloom shader data");

    // Vertex Buffer for fullscreen quad
    eastl::vector<Vert_PosTex> quadVertices = {
        Vert_PosTex{Vec3f(-1.0f, -1.0f, 0.0f), Vec2f(0.0f, 1.0f)},
        Vert_PosTex{Vec3f(1.f, -1.f, 0.0f), Vec2f(1.0f, 1.0f)},
        Vert_PosTex{Vec3f(-1.f, 1.f, 0.0f), Vec2f(0.0f, 0.0f)},
        Vert_PosTex{Vec3f(1.f, 1.f, 0.0f), Vec2f(1.0f, 0.0f)}
    };
    pp.fullScreenQuad = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vert_PosTex), quadVertices.data(), "Fullscreen quad");

    pp.fullScreenTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Wrap, "Fullscreen texture");
}

// ***********************************************************************

void PostProcessingSystem::OnRemovePostProcessing(Scene& scene, EntityID entity)
{
    CPostProcessing& pp = *(scene.Get<CPostProcessing>(entity));

	GfxDevice::FreeConstBuffer(pp.postProcessDataBuffer);
	GfxDevice::FreeConstBuffer(pp.bloomDataBuffer);
	GfxDevice::FreeVertexBuffer(pp.fullScreenQuad);
	GfxDevice::FreeSampler(pp.fullScreenTextureSampler);
	GfxDevice::FreeRenderTarget(pp.blurredFrame[0]);
	GfxDevice::FreeRenderTarget(pp.blurredFrame[1]);
}

// ***********************************************************************

void PostProcessingSystem::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
    PROFILE();
    GFX_SCOPED_EVENT("Doing post processing");	
    TextureHandle preProcessedFrame = GfxDevice::MakeResolvedTexture(ctx.backBuffer);

    for (EntityID ent : SceneView<CPostProcessing>(scene))
    {
        CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

        GfxDevice::BindRenderTarget(pp->blurredFrame[0]);
        GfxDevice::ClearRenderTarget(pp->blurredFrame[0], { 0.0f, 0.f, 0.f, 1.0f }, true, true);
        GfxDevice::SetViewport(0.f, 0.f, GameRenderer::GetWidth() / 2.0f, GameRenderer::GetHeight() / 2.0f);
        GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

        // Bind bloom shader data
        Shader* pShader = AssetDB::GetAsset<Shader>(pp->bloomShader);
        GfxDevice::BindProgram(pShader->program);
        GfxDevice::BindVertexBuffers(1, &pp->fullScreenQuad);
        GfxDevice::BindSampler(pp->fullScreenTextureSampler, ShaderType::Pixel, 0);

        CPostProcessing::BloomShaderData bloomData;
        bloomData.resolution = Vec2f(900, 500);
        
        // Iteratively calculate bloom
        int blurIterations = 8;
        for (int i = 0; i < blurIterations; ++i)
        {
            // Bind data
            float radius = float(blurIterations - i - 1) * 0.04f;
            bloomData.direction = i % 2 == 0 ? Vec2f(radius, 0.0f) : Vec2f(0.0f, radius);
            GfxDevice::BindConstantBuffer(pp->bloomDataBuffer, &bloomData, ShaderType::Pixel, 0);

            if (i == 0)
            {
                // First iteration, bind the plain, preprocessed frame
                GfxDevice::BindTexture(preProcessedFrame, ShaderType::Pixel, 0);
            }
            else
            {
                GfxDevice::UnbindRenderTarget(pp->blurredFrame[(i + 1) % 2]);
                TextureHandle tex = GfxDevice::GetTexture(pp->blurredFrame[(i + 1) % 2]);
                GfxDevice::BindTexture(tex, ShaderType::Pixel, 0);
                GfxDevice::BindRenderTarget(pp->blurredFrame[i % 2]);
                GfxDevice::ClearRenderTarget(pp->blurredFrame[i % 2], { 0.0f, 0.f, 0.f, 1.0f }, true, true);
            }

            GfxDevice::Draw(4, 0);
        }

        // Now we'll actually render onto the backbuffer and do our final post process stage
        GameRenderer::SetBackBufferActive();
        GameRenderer::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f }, true, true);
        GfxDevice::SetViewport(0, 0, GameRenderer::GetWidth(), GameRenderer::GetHeight());

        Shader* pPPShader = AssetDB::GetAsset<Shader>(pp->postProcessShader);
        GfxDevice::BindProgram(pPPShader->program);
        GfxDevice::BindVertexBuffers(1, &pp->fullScreenQuad);

        GfxDevice::BindTexture(preProcessedFrame, ShaderType::Pixel, 0);
        TextureHandle blurFrameTex = GfxDevice::GetTexture(pp->blurredFrame[1]);
        GfxDevice::BindTexture(blurFrameTex, ShaderType::Pixel, 1);

        GfxDevice::BindSampler(pp->fullScreenTextureSampler, ShaderType::Pixel, 0);
        
        CPostProcessing::PostProcessShaderData ppData;
        ppData.resolution = Vec2f(GameRenderer::GetWidth(), GameRenderer::GetHeight());
        ppData.time = float(SDL_GetTicks()) / 1000.0f;
        GfxDevice::BindConstantBuffer(pp->postProcessDataBuffer, &ppData, ShaderType::Pixel, 0);

        // Draw post processed frame
        GfxDevice::Draw(4, 0);

    }
    GfxDevice::FreeTexture(preProcessedFrame);
}

// ***********************************************************************

void PostProcessingSystem::OnWindowResize(Scene& scene, float newWidth, float newHeight)
{
    for (EntityID ent : SceneView<CPostProcessing>(scene))
    {
        CPostProcessing& pp = *(scene.Get<CPostProcessing>(ent));
        for (int i = 0; i < 2; ++i)
        {
            GfxDevice::FreeRenderTarget(pp.blurredFrame[i]);
            pp.blurredFrame[i] = GfxDevice::CreateRenderTarget(GameRenderer::GetWidth() / 2.0f, GameRenderer::GetHeight() / 2.0f, 1, eastl::string().sprintf("Blurred frame %i", i));
        }
    }
}