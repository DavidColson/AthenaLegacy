#include "GameRenderer.h"

#include "Engine.h"
#include "Scene.h"
#include "GraphicsDevice.h"
#include "PostProcessing.h"
#include "DebugDraw.h"
#include "SceneDrawSystem.h"
#include "SpriteDrawSystem.h"
#include "Editor/Editor.h"
#include "Vec2.h"
#include "Maths.h"
#include "Systems.h"

#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

REFLECT_ENUM_BEGIN(ProjectionMode)
REFLECT_ENUMERATOR(Orthographic)
REFLECT_ENUMERATOR(Perspective)
REFLECT_ENUM_END()

REFLECT_COMPONENT_BEGIN(CCamera)
REFLECT_MEMBER(fov)
REFLECT_MEMBER(horizontalAngle)
REFLECT_MEMBER(verticalAngle)
REFLECT_MEMBER(projection)
REFLECT_END()

namespace
{
    TextureHandle resolvedGameFrame;
    RenderTargetHandle gameRenderTarget;
    Vec2f gameWindowSize;
    SceneDrawSystem* pSceneDrawSystem;

    bool postProcessing{ false };

    eastl::vector<IWorldSystem*> opaqueRenderPassSystems;
    eastl::vector<IWorldSystem*> transparentRenderPassSystems;
}

void GameRenderer::SetSceneDrawSystem(SceneDrawSystem* system)
{
    pSceneDrawSystem = system;
}

SceneDrawSystem* GameRenderer::GetSceneDrawSystem()
{
    return pSceneDrawSystem;
}

// ***********************************************************************

void GameRenderer::Initialize(float width, float height, bool postProcessingEnabled)
{
	DebugDraw::Initialize();

    gameRenderTarget = GfxDevice::CreateRenderTarget(width, height, Engine::GetConfig().multiSamples, "Game Render Target");
    gameWindowSize = Vec2f(width, height);

    if (postProcessingEnabled)
        PostProcessing::Initialize();

    postProcessing = postProcessingEnabled;
}

// ***********************************************************************

void GameRenderer::RegisterRenderSystemOpaque(IWorldSystem* pSystem)
{
    opaqueRenderPassSystems.push_back(pSystem);
}

// ***********************************************************************

void GameRenderer::RegisterRenderSystemTransparent(IWorldSystem* pSystem)
{
    transparentRenderPassSystems.push_back(pSystem);
}

// ***********************************************************************

void GameRenderer::UnregisterRenderSystemOpaque(IWorldSystem* pSystem)
{
    eastl::vector<IWorldSystem*>::iterator found = eastl::find(opaqueRenderPassSystems.begin(), opaqueRenderPassSystems.end(), pSystem);
	if (found != opaqueRenderPassSystems.end())
	{
		opaqueRenderPassSystems.erase(found);
	}
}

// ***********************************************************************

void GameRenderer::UnregisterRenderSystemTransparent(IWorldSystem* pSystem)
{
    eastl::vector<IWorldSystem*>::iterator found = eastl::find(transparentRenderPassSystems.begin(), transparentRenderPassSystems.end(), pSystem);
	if (found != transparentRenderPassSystems.end())
	{
		transparentRenderPassSystems.erase(found);
	}
}

// ***********************************************************************

TextureHandle GameRenderer::DrawFrame(Scene& scene, UpdateContext& ctx)
{
    GfxDevice::BindRenderTarget(gameRenderTarget);
    GfxDevice::ClearRenderTarget(gameRenderTarget, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
    GfxDevice::SetViewport(0.0f, 0.0f, gameWindowSize.x, gameWindowSize.y);

    FrameContext frameCtx;
    frameCtx.backBuffer = gameRenderTarget;
    frameCtx.view = Matrixf::Identity();
    frameCtx.screenDimensions = gameWindowSize;
    frameCtx.projection = Matrixf::Orthographic(0.f, gameWindowSize.x, 0.0f, gameWindowSize.y, -1.0f, 200.0f);
    frameCtx.camWorldPosition = Vec3f(0.0f);

	for (EntityID cams : SceneIterator<CCamera, CTransform>(scene))
	{
        // NOTE This is wrong, use global transform for camera, local may not be correct if parented to something
		CCamera* pCam = scene.Get<CCamera>(cams);
		CTransform* pTrans = scene.Get<CTransform>(cams);
		Matrixf translate = Matrixf::MakeTranslation(-pTrans->localPos);
		Quatf rotation = Quatf::MakeFromEuler(pTrans->localRot);
		frameCtx.view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;
        frameCtx.camWorldPosition = Vec3f(pTrans->localPos);

        if (pCam->projection == ProjectionMode::Perspective)
		    frameCtx.projection = Matrixf::Perspective(gameWindowSize.x, gameWindowSize.y, 0.1f, 100.0f, pCam->fov);
        else if (pCam->projection == ProjectionMode::Orthographic)
		    frameCtx.projection = Matrixf::Orthographic(0.f, gameWindowSize.x, 0.0f, gameWindowSize.y, -1.0f, 200.0f);
	}

    SceneRenderPassOpaque(scene, ctx, frameCtx);
    SceneRenderPassTransparent(scene, ctx, frameCtx);
    
    // Post processing, always last
    if (postProcessing)
        PostProcessing::OnFrame(frameCtx, ctx.deltaTime);

    GfxDevice::UnbindRenderTarget(gameRenderTarget);

    // Since the game is multisampled, we have to resolve the multisampled render target to a texture before we're done
    GfxDevice::FreeTexture(resolvedGameFrame);
    resolvedGameFrame = GfxDevice::MakeResolvedTexture(gameRenderTarget);

    return resolvedGameFrame; 
}

// ***********************************************************************

void GameRenderer::SceneRenderPassOpaque(Scene& scene, UpdateContext& ctx, FrameContext& frameCtx)
{
    // Opaque things
    for (IWorldSystem* pSystem : opaqueRenderPassSystems)
    {
        pSystem->Draw(ctx, frameCtx);
    }
    DebugDraw::OnFrame(ctx, frameCtx);
}

// ***********************************************************************

void GameRenderer::SceneRenderPassTransparent(Scene& scene, UpdateContext& ctx, FrameContext& frameCtx)
{
    // Things that have transparency
    for (IWorldSystem* pSystem : transparentRenderPassSystems)
    {
        pSystem->Draw(ctx, frameCtx);
    }
}

// ***********************************************************************

void GameRenderer::OnFrameEnd(Scene& scene, float deltaTime)
{
    DebugDraw::OnFrameEnd(deltaTime);
}

// ***********************************************************************

void GameRenderer::Destroy()
{
    GfxDevice::FreeRenderTarget(gameRenderTarget);
    GfxDevice::FreeTexture(resolvedGameFrame);

    DebugDraw::Destroy();

    if (postProcessing)
        PostProcessing::Destroy();
}

// ***********************************************************************

float GameRenderer::GetWidth()
{
    return gameWindowSize.x;
}

// ***********************************************************************

float GameRenderer::GetHeight()
{
    return gameWindowSize.y;
}

// ***********************************************************************

void GameRenderer::SetBackBufferActive()
{
     GfxDevice::BindRenderTarget(gameRenderTarget);
}

void GameRenderer::ClearBackBuffer(eastl::array<float, 4> color, bool clearDepth, bool clearStencil)
{
    GfxDevice::ClearRenderTarget(gameRenderTarget, color, clearDepth, clearStencil);
}

// ***********************************************************************

TextureHandle GameRenderer::GetLastRenderedFrame()
{
    return resolvedGameFrame;
}

// ***********************************************************************

Vec2f GameRenderer::GetIdealFrameSize(float parentWidth, float parentHeight)
{
    const EngineConfig& config = Engine::GetConfig();
    float baseGameAspectRatio = config.baseGameResolution.x / config.baseGameResolution.y;
    float windowAspectRatio = parentWidth / parentHeight;

    Vec2f idealFrameSize;
    switch (config.resolutionStretchMode)
	{
	case ResolutionStretchMode::NoStretch: 
		idealFrameSize = Vec2f(parentWidth, parentHeight);
		break;
	case ResolutionStretchMode::IgnoreAspect:
	case ResolutionStretchMode::KeepAspect:
		idealFrameSize = Vec2f(config.baseGameResolution.x, config.baseGameResolution.y);
		break;
	case ResolutionStretchMode::KeepWidth:
		idealFrameSize = Vec2f(config.baseGameResolution.x, config.baseGameResolution.y / clamp(windowAspectRatio / baseGameAspectRatio, 0.0f, 1.0f));
		break;
	case ResolutionStretchMode::KeepHeight:
		idealFrameSize = Vec2f(config.baseGameResolution.x / clamp(baseGameAspectRatio / windowAspectRatio, 0.0f, 1.0f), config.baseGameResolution.y);
        break;
    case ResolutionStretchMode::Expand:
		idealFrameSize = Vec2f(config.baseGameResolution.x / clamp(baseGameAspectRatio / windowAspectRatio, 0.0f, 1.0f), config.baseGameResolution.y / clamp(windowAspectRatio / baseGameAspectRatio, 0.0f, 1.0f));
        break;
	default:
		break;
	}

    return idealFrameSize;
}

// ***********************************************************************

void GameRenderer::ResizeGameFrame(float newWidth, float newHeight)
{
    gameWindowSize = Vec2f(newWidth, newHeight);
    GfxDevice::FreeRenderTarget(gameRenderTarget);
    gameRenderTarget = GfxDevice::CreateRenderTarget(gameWindowSize.x, gameWindowSize.y, Engine::GetConfig().multiSamples, "Game Render Target");
    PostProcessing::OnWindowResize(gameWindowSize.x, gameWindowSize.y);
}
