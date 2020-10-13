#pragma once

struct Scene;
struct TextureHandle;

namespace RenderSystem
{
    void Initialize(float width, float height);
    void OnSceneCreate(Scene& scene);
    void OnFrame(Scene& scene, float deltaTime);
    void Destroy();

    TextureHandle GetGameFrame();

    float GetWidth();
    float GetHeight();

    void ResizeGameFrame(Scene& scene, float newWidth, float newHeight);
}