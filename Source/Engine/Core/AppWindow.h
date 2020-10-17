#pragma once

#include "GraphicsDevice.h"

namespace AppWindow
{
    void Create(float initialWidth, float initialHeight, const eastl::string& windowName);
    
    void RenderToWindow(TextureHandle frame);

    void Resize(float width, float height);

    float GetWidth();
    float GetHeight();

    SDL_Window* GetSDLWindow();

    void Destroy();
}