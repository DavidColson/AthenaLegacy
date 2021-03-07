#pragma once

#include "EASTL/array.h"

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"

class ISystem;
struct SceneDrawSystem;
struct Scene;
struct TextureHandle;

enum class ProjectionMode
{
    Orthographic,
    Perspective
};
REFLECT_ENUM(ProjectionMode)

struct CCamera
{
    float fov{ 60.0f };
    float horizontalAngle{ 0.0f };
    float verticalAngle{ 0.0f };
    ProjectionMode projection{ ProjectionMode::Orthographic };

    REFLECT()
};

struct FrameContext
{
    RenderTargetHandle backBuffer;
    Matrixf projection;
    Matrixf view;
    Vec3f camWorldPosition;
    Vec2f screenDimensions;
};

namespace GameRenderer
{
    void Initialize(float width, float height);
    void OnSceneCreate(Scene& scene);
    TextureHandle DrawFrame(Scene& scene, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
    void Destroy();

    void RegisterRenderSystemOpaque(ISystem* pSystem);
    void RegisterRenderSystemTransparent(ISystem* pSystem);

    void UnregisterRenderSystemOpaque(ISystem* pSystem);
    void UnregisterRenderSystemTransparent(ISystem* pSystem);

    void SceneRenderPassOpaque(Scene& scene, FrameContext& ctx, float deltaTime);
    void SceneRenderPassTransparent(Scene& scene, FrameContext& ctx, float deltaTime);

    void SetSceneDrawSystem(SceneDrawSystem* system);
    SceneDrawSystem* GetSceneDrawSystem();

    void SetBackBufferActive();
    void ClearBackBuffer(eastl::array<float, 4> color, bool clearDepth, bool clearStencil);
    TextureHandle GetLastRenderedFrame();

    float GetWidth();
    float GetHeight();
    
    Vec2f GetIdealFrameSize(float parentWidth, float parentHeight);
    void ResizeGameFrame(float newWidth, float newHeight);
}