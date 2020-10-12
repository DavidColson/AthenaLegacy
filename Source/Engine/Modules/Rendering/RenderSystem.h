#pragma once

struct Scene;
struct TextureHandle;

namespace RenderSystem
{
    void Initialize();
    void OnSceneCreate(Scene& scene);
    void PreUpdate(Scene& scene, float deltaTime);
    void OnFrame(Scene& scene, float deltaTime);
    void Destroy();

    TextureHandle GetGameFrame();

    float GetGameViewWidth();
    float GetGameViewHeight();

    void OnWindowResize(Scene& scene, float newWidth, float newHeight);
}