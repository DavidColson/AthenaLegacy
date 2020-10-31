#include "SceneView.h"

#include "GraphicsDevice.h"
#include "Rendering/GameRenderer.h"
#include "Scene.h"

#include "Rendering/ParticlesSystem.h"
#include "Rendering/FontSystem.h"
#include "Rendering/DebugDraw.h"
#include "Rendering/ShapesSystem.h"
#include "Rendering/SceneDrawSystem.h"
#include "Rendering/SpriteDrawSystem.h"

#include <ImGui/imgui.h>

namespace
{
    RenderTargetHandle renderTarget;
    TextureHandle lastFrame;
    Vec2f windowSize{ Vec2f(0.0f, 0.0f) };
    CCamera camera;
    CTransform cameraTransform;
}

SceneView::SceneView()
{
    menuName = "Scene View";
}

void SceneView::Update(Scene& scene, float deltaTime)
{
    ImGui::Begin("Scene", &open);

    if (windowSize != Vec2f(ImGui::GetContentRegionAvail()))
	{
		windowSize = ImGui::GetContentRegionAvail();
        GfxDevice::FreeRenderTarget(renderTarget);
        renderTarget = GfxDevice::CreateRenderTarget(windowSize.x, windowSize.y, 4, "SceneView Render Target");
    }

    // Render the scene view
    GfxDevice::BindRenderTarget(renderTarget);
    GfxDevice::ClearRenderTarget(renderTarget, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
    GfxDevice::SetViewport(0.0f, 0.0f, windowSize.x, windowSize.y);

    FrameContext context;
    context.backBuffer = renderTarget;
    context.view = Matrixf::Identity();
    context.projection = Matrixf::Orthographic(0.f, windowSize.x, 0.0f, windowSize.y, -1.0f, 200.0f);

    Matrixf translate = Matrixf::MakeTranslation(-cameraTransform.localPos);
    Quatf rotation = Quatf::MakeFromEuler(cameraTransform.localRot);
    context.view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;

    if (camera.projection == ProjectionMode::Perspective)
        context.projection = Matrixf::Perspective(windowSize.x, windowSize.y, 0.1f, 100.0f, camera.fov);
    else if (camera.projection == ProjectionMode::Orthographic)
        context.projection = Matrixf::Orthographic(0.f, windowSize.x, 0.0f, windowSize.y, -1.0f, 200.0f);

    SceneDrawSystem::OnFrame(scene, context, deltaTime);
    Shapes::OnFrame(scene, context, deltaTime);
    ParticlesSystem::OnFrame(scene, context, deltaTime);
    FontSystem::OnFrame(scene, context, deltaTime);
    SpriteDrawSystem::OnFrame(scene, context, deltaTime);
    DebugDraw::OnFrame(scene, context, deltaTime);

    GfxDevice::UnbindRenderTarget(renderTarget);
    
    GfxDevice::FreeTexture(lastFrame);
    lastFrame = GfxDevice::MakeResolvedTexture(renderTarget);
    
    ImVec2 imageDrawSize = ImVec2(windowSize.x, windowSize.y);
	ImVec2 imageDrawLocation = ImVec2(ImGui::GetCursorPosX() + windowSize.x * 0.5f - imageDrawSize.x * 0.5f, ImGui::GetCursorPosY() + windowSize.y * 0.5f - imageDrawSize.y * 0.5f);

    ImGui::Image(GfxDevice::GetImGuiTextureID(lastFrame), imageDrawSize);

    ImGui::End();
}

void SceneView::OnEditorResize(Vec2f newSize)
{
    windowSize = newSize;
}