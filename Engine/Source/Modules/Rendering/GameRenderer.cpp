#include "GameRenderer.h"

#include "Engine.h"
#include "Scene.h"
#include "GraphicsDevice.h"
#include "ParticlesSystem.h"
#include "PostProcessingSystem.h"
#include "FontSystem.h"
#include "DebugDraw.h"
#include "ShapesSystem.h"
#include "SceneDrawSystem.h"
#include "SpriteDrawSystem.h"
#include "Editor/Editor.h"
#include "Vec2.h"
#include "Maths.h"

#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

namespace
{
    TextureHandle resolvedGameFrame;
    RenderTargetHandle gameRenderTarget;
    Vec2f gameWindowSize;
}

// ***********************************************************************

void GameRenderer::Initialize(float width, float height)
{
    Shapes::Initialize();
	DebugDraw::Initialize();
	FontSystem::Initialize();
    SpriteDrawSystem::Initialize();

    gameRenderTarget = GfxDevice::CreateRenderTarget(width, height, 4, "Game Render Target");

    gameWindowSize = Vec2f(width, height);
}

// ***********************************************************************

void GameRenderer::OnSceneCreate(Scene& scene)
{
    SceneDrawSystem::OnSceneCreate(scene);

    scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnAdd, ParticlesSystem::OnAddEmitter);
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnRemove, ParticlesSystem::OnRemoveEmitter);

	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnAdd, PostProcessingSystem::OnAddPostProcessing);
	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnRemove, PostProcessingSystem::OnRemovePostProcessing);
}

// ***********************************************************************

TextureHandle GameRenderer::DrawFrame(Scene& scene, float deltaTime)
{
    GfxDevice::BindRenderTarget(gameRenderTarget);
    GfxDevice::ClearRenderTarget(gameRenderTarget, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
    GfxDevice::SetViewport(0.0f, 0.0f, gameWindowSize.x, gameWindowSize.y);

    FrameContext context;
    context.backBuffer = gameRenderTarget;
    context.view = Matrixf::Identity();
    context.projection = Matrixf::Orthographic(0.f, GameRenderer::GetWidth(), 0.0f, GameRenderer::GetHeight(), -1.0f, 10.0f);

	for (EntityID cams : SceneView<CCamera, CTransform>(scene))
	{
		CCamera* pCam = scene.Get<CCamera>(cams);
		CTransform* pTrans = scene.Get<CTransform>(cams);
		Matrixf translate = Matrixf::MakeTranslation(-pTrans->localPos);
		Quatf rotation = Quatf::MakeFromEuler(pTrans->localRot);
		context.view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;
		context.projection = Matrixf::Perspective(GameRenderer::GetWidth(), GameRenderer::GetHeight(), 0.1f, 100.0f, pCam->fov);
	}

    SceneDrawSystem::OnFrame(scene, context, deltaTime);
    Shapes::OnFrame(scene, context, deltaTime);
    ParticlesSystem::OnFrame(scene, context, deltaTime);
    FontSystem::OnFrame(scene, context, deltaTime);
    SpriteDrawSystem::OnFrame(scene, context, deltaTime);
    DebugDraw::OnFrame(scene, context, deltaTime);
    PostProcessingSystem::OnFrame(scene, context, deltaTime);

    GfxDevice::UnbindRenderTarget(gameRenderTarget);

    // Since the game is multisampled, we have to resolve the multisampled render target to a texture before we're done
    GfxDevice::FreeTexture(resolvedGameFrame);
    resolvedGameFrame = GfxDevice::MakeResolvedTexture(gameRenderTarget);

    return resolvedGameFrame; 
}

// ***********************************************************************

void GameRenderer::Destroy()
{
    DebugDraw::Destroy();
	Shapes::Destroy();
	FontSystem::Destroy();
	SpriteDrawSystem::Destroy();
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
	case 0: 
		idealFrameSize = Vec2f(parentWidth, parentHeight);
		break;
	case 1:
	case 2:
		idealFrameSize = Vec2f(config.baseGameResolution.x, config.baseGameResolution.y);
		break;
	case 3:
		idealFrameSize = Vec2f(config.baseGameResolution.x, config.baseGameResolution.y / clamp(windowAspectRatio / baseGameAspectRatio, 0.0f, 1.0f));
		break;
	case 4:
		idealFrameSize = Vec2f(config.baseGameResolution.x / clamp(baseGameAspectRatio / windowAspectRatio, 0.0f, 1.0f), config.baseGameResolution.y);
        break;
    case 5:
		idealFrameSize = Vec2f(config.baseGameResolution.x / clamp(baseGameAspectRatio / windowAspectRatio, 0.0f, 1.0f), config.baseGameResolution.y / clamp(windowAspectRatio / baseGameAspectRatio, 0.0f, 1.0f));
        break;
	default:
		break;
	}

    return idealFrameSize;
}

// ***********************************************************************

void GameRenderer::ResizeGameFrame(Scene& scene, float newWidth, float newHeight)
{
    gameWindowSize = Vec2f(newWidth, newHeight);
    GfxDevice::FreeRenderTarget(gameRenderTarget);
    gameRenderTarget = GfxDevice::CreateRenderTarget(gameWindowSize.x, gameWindowSize.y, 4, "Game Render Target");

    PostProcessingSystem::OnWindowResize(scene, gameWindowSize.x, gameWindowSize.y);
}
