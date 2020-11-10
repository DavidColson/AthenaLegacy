#pragma once

#include "Vec3.h"

struct Scene;
struct FrameContext;

namespace GfxDraw
{
    void Line(Vec3f start, Vec3f end, float thickness);

    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};