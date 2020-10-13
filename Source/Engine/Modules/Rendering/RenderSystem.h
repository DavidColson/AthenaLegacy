#pragma once

struct Scene;
struct TextureHandle;

namespace RenderSystem
{
    void Initialize(float width, float height);
    void OnSceneCreate(Scene& scene);
    TextureHandle DrawFrame(Scene& scene, float deltaTime);
    void Destroy();

    TextureHandle GetCurrentFrame();
    float GetWidth();
    float GetHeight();

    void ResizeGameFrame(Scene& scene, float newWidth, float newHeight);
}