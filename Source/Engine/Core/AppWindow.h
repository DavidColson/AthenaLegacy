#pragma once

#include "GraphicsDevice.h"

namespace AppWindow
{
    void Create(float initialWidth, float initialHeight);
    
    void RenderToWindow(TextureHandle frame);

    void Resize(float width, float height);

    float GetWidth();
    float GetHeight();

    SDL_Window* GetSDLWindow();

    void Destroy();
}