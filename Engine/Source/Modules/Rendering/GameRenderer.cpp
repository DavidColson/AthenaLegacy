#include "GameRenderer.h"

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

    SceneDrawSystem::OnFrame(scene, deltaTime);
    Shapes::OnFrame(scene, deltaTime);
    ParticlesSystem::OnFrame(scene, deltaTime);
    FontSystem::OnFrame(scene, deltaTime);
    SpriteDrawSystem::OnFrame(scene, deltaTime);
    DebugDraw::OnFrame(scene, deltaTime);
    PostProcessingSystem::OnFrame(scene, deltaTime);

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

TextureHandle GameRenderer::NewResolvedBackbuffer()
{
    return GfxDevice::MakeResolvedTexture(gameRenderTarget);
}

// ***********************************************************************

TextureHandle GameRenderer::GetLastRenderedFrame()
{
    return resolvedGameFrame;
}

// ***********************************************************************

void GameRenderer::ResizeGameFrame(Scene& scene, float newWidth, float newHeight)
{
    gameWindowSize = Vec2f(newWidth, newHeight);

    GfxDevice::FreeRenderTarget(gameRenderTarget);
    gameRenderTarget = GfxDevice::CreateRenderTarget(newWidth, newHeight, 4, "Game Render Target");

    PostProcessingSystem::OnWindowResize(scene, newWidth, newHeight);
}