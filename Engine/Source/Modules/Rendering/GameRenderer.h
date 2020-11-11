#pragma once

#include "EASTL/array.h"

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"

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
};

namespace GameRenderer
{
    void Initialize(float width, float height);
    void OnSceneCreate(Scene& scene);
    TextureHandle DrawFrame(Scene& scene, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
    void Destroy();

    void SetBackBufferActive();
    void ClearBackBuffer(eastl::array<float, 4> color, bool clearDepth, bool clearStencil);
    TextureHandle GetLastRenderedFrame();

    float GetWidth();
    float GetHeight();
    
    Vec2f GetIdealFrameSize(float parentWidth, float parentHeight);
    void ResizeGameFrame(Scene& scene, float newWidth, float newHeight);
}