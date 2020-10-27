#pragma once

#include "EASTL/array.h"

#include "Vec2.h"

struct Scene;
struct TextureHandle;

namespace GameRenderer
{
    void Initialize(float width, float height);
    void OnSceneCreate(Scene& scene);
    TextureHandle DrawFrame(Scene& scene, float deltaTime);
    void Destroy();

    // Game screen back buffer manipulation. Useful for post processing the game screen but not the whole screen
    TextureHandle NewResolvedBackbuffer();
    void SetBackBufferActive();
    void ClearBackBuffer(eastl::array<float, 4> color, bool clearDepth, bool clearStencil);
    TextureHandle GetLastRenderedFrame();

    float GetWidth();
    float GetHeight();
    
    Vec2f GetIdealFrameSize(float parentWidth, float parentHeight);
    void ResizeGameFrame(Scene& scene, float newWidth, float newHeight);
}