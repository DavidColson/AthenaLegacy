#include "RenderSystem.h"

#include "Scene.h"
#include "GraphicsDevice.h"
#include "ParticlesSystem.h"
#include "PostProcessingSystem.h"
#include "FontSystem.h"
#include "DebugDraw.h"
#include "ShapesSystem.h"
#include "SceneDrawSystem.h"

#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

void RenderSystem::Initialize()
{
    Shapes::Initialize();
	DebugDraw::Initialize();
	FontSystem::Initialize();
}

void RenderSystem::OnSceneCreate(Scene& scene)
{
    SceneDrawSystem::OnSceneCreate(scene);

    scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnAdd, ParticlesSystem::OnAddEmitter);
	scene.RegisterReactiveSystem<CParticleEmitter>(Reaction::OnRemove, ParticlesSystem::OnRemoveEmitter);

	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnAdd, PostProcessingSystem::OnAddPostProcessing);
	scene.RegisterReactiveSystem<CPostProcessing>(Reaction::OnRemove, PostProcessingSystem::OnRemovePostProcessing);
}

void RenderSystem::PreUpdate(Scene& scene, float deltaTime)
{
    ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void RenderSystem::OnFrame(Scene& scene, float deltaTime)
{
    GfxDevice::SetBackBufferActive();
    GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
    GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

    SceneDrawSystem::OnFrame(scene, deltaTime);
    Shapes::OnFrame(scene, deltaTime);
    ParticlesSystem::OnFrame(scene, deltaTime);
    FontSystem::OnFrame(scene, deltaTime);
    DebugDraw::OnFrame(scene, deltaTime);
    PostProcessingSystem::OnFrame(scene, deltaTime);

    {
        GFX_SCOPED_EVENT("Drawing imgui");
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    GfxDevice::PresentBackBuffer();
    GfxDevice::ClearRenderState();
    GfxDevice::PrintQueuedDebugMessages();
}

void RenderSystem::Destroy()
{
    DebugDraw::Destroy();
	Shapes::Destroy();
	FontSystem::Destroy();
}

void RenderSystem::OnWindowResize(Scene& scene, float newWidth, float newHeight)
{
    PostProcessingSystem::OnWindowResize(scene, newWidth, newHeight);
}
