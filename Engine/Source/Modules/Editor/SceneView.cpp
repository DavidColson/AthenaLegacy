#include "SceneView.h"

#include "Input/Input.h"
#include "AppWindow.h"

#include "SceneQueries.h"

#include "Rendering/ParticlesSystem.h"
#include "Rendering/FontSystem.h"
#include "Rendering/SceneDrawSystem.h"
#include "Rendering/SpriteDrawSystem.h"
#include "Rendering/GfxDraw.h"

#include <ImGui/imgui.h>
#include <SDL.h>

SceneView::SceneView()
{
    menuName = "Scene View";
    if (isIn2DMode)
        camera.projection = ProjectionMode::Orthographic;
    else
        camera.projection = ProjectionMode::Perspective;
}

SceneView::~SceneView()
{
    GfxDevice::FreeRenderTarget(renderTarget);
    GfxDevice::FreeTexture(lastFrame);
}

bool SceneView::OnEvent(SDL_Event* event)
{
    if (isHovered)
    {
        switch (event->type)
        {
        case SDL_KEYDOWN:
            switch (event->key.keysym.scancode)
            {
            case SDL_SCANCODE_A: controls.left = true; break;
            case SDL_SCANCODE_D: controls.right = true; break;
            case SDL_SCANCODE_W: controls.forward = true; break;
            case SDL_SCANCODE_S: controls.backward = true; break;
            case SDL_SCANCODE_SPACE: controls.up = true; break;
            case SDL_SCANCODE_LCTRL: controls.down = true; break;
            default: break;
            }
            break;
        case SDL_KEYUP:
            switch (event->key.keysym.scancode)
            {
            case SDL_SCANCODE_A: controls.left = false; break;
            case SDL_SCANCODE_D: controls.right = false; break;
            case SDL_SCANCODE_W: controls.forward = false; break;
            case SDL_SCANCODE_S: controls.backward = false; break;
            case SDL_SCANCODE_SPACE: controls.up = false; break;
            case SDL_SCANCODE_LCTRL: controls.down = false; break;
            default: break;
            }
            break;
        case SDL_MOUSEMOTION:
            controls.mouseXRel = (float)event->motion.xrel;
            controls.mouseYRel = (float)event->motion.yrel;
            controls.mouseX = (float)event->motion.x;
            controls.mouseY = (float)event->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:

            // if left mouse button down
            // Then call a special function (in maybe a new file like SceneQueries)
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                // Calculate ray, save for doing in update function
                bPendingLeftClickRay = true;
                

                rayStart = cameraTransform3D.localPos;
                Matrixf camRot = Matrixf::MakeRotation(cameraTransform3D.localRot);

                // Need to get a location on the screen as a normalized coordinate. Then multiply by inverse/view matrix to get the direction
                Vec4f ray = Vec4f((controls.localMouseX / windowSize.x) * 2.0f - 1.0f, 1.0f - (controls.localMouseY / windowSize.y) * 2.0f, 1.0f, 1.0f);

                // BUG: Not correct if camera parented to something
                Quatf rotation = Quatf::MakeFromEuler(cameraTransform3D.localRot);
                Matrixf view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector());
                Matrixf projection = Matrixf::Perspective(windowSize.x, windowSize.y, 0.1f, 100.0f, camera.fov);

                Vec4f rayEye = projection.GetInverse() * ray;
                rayEye = Vec4f(rayEye.x, rayEye.y, -1.0f, 1.0f);
                Vec4f rayWorld = view.GetInverse() * rayEye;
                rayDir = Vec3f(rayWorld.x, rayWorld.y, rayWorld.z).GetNormalized();
            }
            if (event->button.button == SDL_BUTTON_RIGHT)
            {
                controls.rightMouse = true;
                if (isIn2DMode)
                {
                    SDL_GetMouseState(&mouseDragStart.x, &mouseDragStart.y);
                    cameraPositionStart = cameraTransform2D.localPos;
                }
                else
                {
                    SDL_GetGlobalMouseState(&relativeMouseStartLocation.x, &relativeMouseStartLocation.y);
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            } 
            break;
        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_RIGHT)
            {
                controls.rightMouse = false;
                if (!isIn2DMode)
                {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    SDL_WarpMouseGlobal(relativeMouseStartLocation.x, relativeMouseStartLocation.y);
                }
            } 
            break;
        case SDL_MOUSEWHEEL:
            if (isIn2DMode)
            {
                float change = (float)event->wheel.y * -0.2f;
                if ((pixelZoomLevel + change) > 0.01f)
                {
                    pixelZoomLevel += change;
                    Vec3f offset(controls.localMouseX * change, (windowSize.y - controls.localMouseY) * change, 0.0f);
                    cameraTransform2D.localPos -= offset;
                }
            }
            else
            {
                camera.fov -= event->wheel.y * 3.0f;
            }
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

void SceneView::UpdateCameraControls(float deltaTime)
{
    if (controls.rightMouse)
    {
        if (isIn2DMode)
        {
            Vec2i deltaMouse = (Vec2i((int)controls.mouseX, (int)controls.mouseY) - mouseDragStart);
            cameraTransform2D.localPos = cameraPositionStart + Vec3f((float)-deltaMouse.x, (float)deltaMouse.y, 0.0f) * pixelZoomLevel;
        }
        else
        {
            Matrixf toCameraSpace = Quatf::MakeFromEuler(cameraTransform3D.localRot).ToMatrix();
            Vec3f right = toCameraSpace.GetRightVector().GetNormalized();
            if (controls.left)
                cameraTransform3D.localPos -= right * cameraSpeed * deltaTime;
            if (controls.right)
                cameraTransform3D.localPos += right * cameraSpeed * deltaTime;
                
            Vec3f forward = toCameraSpace.GetForwardVector().GetNormalized();
            if (controls.forward)
                cameraTransform3D.localPos -= forward * cameraSpeed * deltaTime;
            if (controls.backward)
                cameraTransform3D.localPos += forward * cameraSpeed * deltaTime;

            Vec3f up = toCameraSpace.GetUpVector().GetNormalized();
            if (controls.up)
                cameraTransform3D.localPos += up * cameraSpeed * deltaTime;
            if (controls.down)
                cameraTransform3D.localPos -= up * cameraSpeed * deltaTime;

            camera.horizontalAngle -= 0.15f * deltaTime * controls.mouseXRel;
            camera.verticalAngle -= 0.15f * deltaTime * controls.mouseYRel;
            cameraTransform3D.localRot = Vec3f(camera.verticalAngle, camera.horizontalAngle, 0.0f);
        }
    }

    controls.mouseXRel = 0.0f;
    controls.mouseYRel = 0.0f;
}

void SceneView::DrawSceneViewHelpers3D()
{
    if (drawGridLines)
    {
        for (int i = 0; i < 100; i++)
        {   
            float staticStart = -50.0f;
            float staticEnd = 50.0f;
            float iterate = i * 1.0f - 50.0f;

            Vec4f color = Vec4f(0.32f, 0.32f, 0.32f, 1.0f);
            if (i % 10 == 0)
                color = Vec4f(0.45f, 0.45f, 0.45f, 1.0f);

            if (iterate == 0.0f)
                continue;
        
            GfxDraw::Paint paint;
            paint.strokeThickness = 0.01f;
            paint.strokeColor = color;

            GfxDraw::SetDrawSpace(GfxDraw::DrawSpace::GameCamera);
            GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::Billboard);
            GfxDraw::SetSortLayer(-10);
            GfxDraw::Line(Vec3f(staticStart, 0.0f, iterate), Vec3f(staticEnd, 0.0f, iterate), paint);
            GfxDraw::Line(Vec3f(iterate, 0.0f, staticStart), Vec3f(iterate, 0.0f, staticEnd), paint);
        }
    }

    if (drawOrigin)
    {
        GfxDraw::SetSortLayer(-9);
        GfxDraw::Paint paint;
        paint.strokeThickness = 0.01f;
        paint.strokeColor = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        GfxDraw::Line(Vec3f(-50.0f, 0.0f, 0.0f), Vec3f(50.0f, 0.0f, 0.0f), paint);
        paint.strokeColor = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
        GfxDraw::Line(Vec3f(0.0f, -50.0f, 0.0f), Vec3f(0.0f, 50.0f, 0.0f), paint);
        paint.strokeColor = Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
        GfxDraw::Line(Vec3f(0.0f, 0.0f, -50.0f), Vec3f(0.0f, 0.0f, 50.0f), paint);
    }
    GfxDraw::SetSortLayer(0);
}

void SceneView::DrawSceneViewHelpers2D()
{
    GfxDraw::SetDrawSpace(GfxDraw::DrawSpace::GameCamera);
    GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::Billboard);
    GfxDraw::SetSortLayer(-10);

    if (drawGridLines)
    {
        for (int i = 0; i < 60; i++)
        {   
            float staticStart = -2000.0f;
            float staticEnd = 4000.0f;
            float iterate = i * 100.0f - 2000.0f;

            GfxDraw::Paint paint;
            paint.strokeThickness = 5.f;
            paint.strokeColor = Vec4f(0.32f, 0.32f, 0.32f, 1.0f);
            if (i % 10 == 0)
                paint.strokeColor = Vec4f(0.45f, 0.45f, 0.45f, 1.0f);

            GfxDraw::Line(Vec3f(staticStart, iterate, -1000.0f), Vec3f(staticEnd, iterate, -1000.0f), paint);
            GfxDraw::Line(Vec3f(iterate, staticStart, -1000.0f), Vec3f(iterate, staticEnd, -1000.0f), paint);
        }
    }
    
    if (drawOrigin)
    {
        GfxDraw::Paint paint;
        paint.strokeThickness = 5.f;
        paint.strokeColor = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        GfxDraw::Line(Vec3f(-5000.0f, 0.0f, -1000.0f), Vec3f(5000.0f, 0.0f, -1000.0f), paint);
        paint.strokeColor = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
        GfxDraw::Line(Vec3f(0.0f, -5000.0f, -1000.0f), Vec3f(0.0f, 5000.0f, -1000.0f), paint);
    }
    
    if (drawFrame)
    {
        float width = GameRenderer::GetWidth();
        float height = GameRenderer::GetHeight();

        GfxDraw::Paint paint;
        paint.strokeThickness = 10.f;
        paint.strokeColor = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
        paint.drawStyle = GfxDraw::DrawStyle::Stroke;

        GfxDraw::Rect(Vec3f(width * 0.5f, height * 0.5f, -1.0f), Vec2f(width + 5.0f, height + 5.0f), Vec4f(0.0f), paint);
    }
}

void SceneView::Update(Scene& scene, float deltaTime)
{
    if (!ImGui::Begin("Scene", &open))
	{
        isHovered = false;
        controls = Controls();
		ImGui::End();
		return;
	}

    UpdateCameraControls(deltaTime);

    if (bPendingLeftClickRay)
    {
        lastHit = SceneQueries::Hit();
        if (SceneQueries::RaycastRenderables(scene, rayStart, rayDir, lastHit))
            Editor::SetSelectedEntity(lastHit.ent);   
        else
            Editor::SetSelectedEntity(EntityID::InvalidID());   
        bPendingLeftClickRay = false;
    }

    // Draw the options bar along the top
    const char* buttonLabel = "3D";
    if (isIn2DMode)
        buttonLabel = "2D";

    if (ImGui::Button(buttonLabel, ImVec2(100.0f, 0.0f)))
    {
        if (isIn2DMode)
        {
            isIn2DMode = false;
            camera.projection = ProjectionMode::Perspective;
        }
        else
        {
            isIn2DMode = true;
            camera.projection = ProjectionMode::Orthographic;
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox("Display Grid Lines", &drawGridLines);
    ImGui::SameLine();
    ImGui::Checkbox("Display Origin", &drawOrigin);
    ImGui::SameLine();
    if (isIn2DMode)
    {
        ImGui::Checkbox("Display Frame", &drawFrame);
        ImGui::SameLine();
    }
    ImGui::PushItemWidth(150);
    ImGui::DragFloat("Camera Fly Speed", &cameraSpeed, deltaTime, 0.0f, 100.0f);




    // Render the scene view

    if (windowSize != Vec2f(ImGui::GetContentRegionAvail()))
	{
		windowSize = ImGui::GetContentRegionAvail();
        GfxDevice::FreeRenderTarget(renderTarget);
        renderTarget = GfxDevice::CreateRenderTarget(windowSize.x, windowSize.y, Engine::GetConfig().multiSamples, "SceneView Render Target");
    }

    GfxDevice::BindRenderTarget(renderTarget);
    GfxDevice::ClearRenderTarget(renderTarget, { 0.25f, 0.25f, 0.25f, 1.0f }, true, true);
    GfxDevice::SetViewport(0.0f, 0.0f, windowSize.x, windowSize.y);

    FrameContext context;
    context.backBuffer = renderTarget;
    context.view = Matrixf::Identity();
    context.screenDimensions = windowSize;

    CTransform activeCamTransform;
    if (isIn2DMode)
    {
        activeCamTransform = cameraTransform2D;
        context.projection = Matrixf::Orthographic(0.f, windowSize.x * pixelZoomLevel, 0.0f, windowSize.y * pixelZoomLevel, -1.0f, 2000.0f);
    }
    else
    {
        activeCamTransform = cameraTransform3D;
        context.projection = Matrixf::Perspective(windowSize.x, windowSize.y, 0.1f, 100.0f, camera.fov);
    }

    // BUG: Not correct if camera parented to something
    Matrixf translate = Matrixf::MakeTranslation(-activeCamTransform.localPos);
    Quatf rotation = Quatf::MakeFromEuler(activeCamTransform.localRot);
    context.view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;
    context.camWorldPosition = Vec3f(activeCamTransform.localPos);

    if (isIn2DMode)
        DrawSceneViewHelpers2D();
    else
        DrawSceneViewHelpers3D();
    
    SceneDrawSystem::OnFrame(scene, context, deltaTime);
    ParticlesSystem::OnFrame(scene, context, deltaTime);
    FontSystem::OnFrame(scene, context, deltaTime);
    GfxDraw::OnFrame(scene, context, deltaTime);
    SpriteDrawSystem::OnFrame(scene, context, deltaTime);

    GfxDevice::UnbindRenderTarget(renderTarget);
    
    GfxDevice::FreeTexture(lastFrame);
    lastFrame = GfxDevice::MakeResolvedTexture(renderTarget);
    


    // Draw the rendered frame into the imgui window

    ImVec2 imageDrawSize = ImVec2(windowSize.x, windowSize.y);
	ImVec2 imageDrawLocation = ImVec2(ImGui::GetCursorPosX() + windowSize.x * 0.5f - imageDrawSize.x * 0.5f, ImGui::GetCursorPosY() + windowSize.y * 0.5f - imageDrawSize.y * 0.5f);
    
    controls.localMouseX = ImGui::GetMousePos().x - (ImGui::GetWindowPos().x + ImGui::GetCursorPos().x);
    controls.localMouseY = ImGui::GetMousePos().y - (ImGui::GetWindowPos().y + ImGui::GetCursorPos().y);

    ImGui::Image(GfxDevice::GetImGuiTextureID(lastFrame), imageDrawSize);

    if (ImGui::IsItemHovered())
    {
		isHovered = true;
    }
	else
    {
        controls = Controls();
		isHovered = false;
    }

    ImGui::End();
}

void SceneView::OnEditorResize(Vec2f newSize)
{
    windowSize = newSize;
}