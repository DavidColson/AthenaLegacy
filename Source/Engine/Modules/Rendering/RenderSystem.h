#pragma once

struct Scene;

namespace RenderSystem
{
    void Initialize();
    void OnSceneCreate(Scene& scene);
    void PreUpdate(Scene& scene, float deltaTime);
    void OnFrame(Scene& scene, float deltaTime);
    void Destroy();

    void OnWindowResize(Scene& scene, float newWidth, float newHeight);
}