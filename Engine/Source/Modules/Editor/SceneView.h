#pragma once

#include "Editor.h"
#include "Vec2.h"
#include "Vec3.h"
#include "GraphicsDevice.h"
#include "Rendering/GameRenderer.h"
#include "Scene.h"

struct SceneView : public EditorTool
{
	SceneView();

    virtual ~SceneView() override;

	virtual void Update(Scene& scene, float deltaTime) override;

    virtual void OnEditorResize(Vec2f newSize) override;

    virtual bool OnEvent(SDL_Event* event) override;

    void UpdateCameraControls(float deltaTime);

    void DrawSceneViewHelpers3D();

    void DrawSceneViewHelpers2D();

    // General Scene view info
    RenderTargetHandle renderTarget;
    TextureHandle lastFrame;
    Vec2f windowSize{ Vec2f(0.0f, 0.0f) };
    bool isIn2DMode = false;
    bool drawGridLines = true;
    bool drawOrigin = true;
    bool drawFrame = true;

    // Scene view camera
    float cameraSpeed = 5.0f;
    float pixelZoomLevel = 1.0f;
    CCamera camera;
    CTransform cameraTransform2D;
    CTransform cameraTransform3D;

    // Scene view control information
    struct Controls
    {
        bool left{ false };
        bool right{ false };
        bool forward{ false };
        bool backward{ false };
        bool up{ false };
        bool down{ false };
        bool rightMouse{ false };
        float mouseXRel{ 0.0f };
        float mouseYRel{ 0.0f };
        float mouseX{ 0.0f };
        float mouseY{ 0.0f };
        float localMouseX{ 0.0f };
        float localMouseY{ 0.0f };
    };
    Controls controls;
    bool isHovered{ false };
    Vec2i relativeMouseStartLocation{ Vec2i(0, 0) };

    // When dragging to move camera we use this data
    Vec2i mouseDragStart{ Vec2i(0, 0) };
    Vec3f cameraPositionStart{ Vec3f(0.0f, 0.0f, 0.0f) };

    bool bPendingLeftClickRay{ false };
    Vec3f rayStart{Vec3f(0.f) };
    Vec3f rayDir{ Vec3f(0.0f) };
};