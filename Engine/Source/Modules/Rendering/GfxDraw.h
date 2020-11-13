#pragma once

#include "Vec3.h"
#include "Vec4.h"

struct Scene;
struct FrameContext;

namespace GfxDraw
{
    void Line(Vec3f start, Vec3f end, Vec4f color, float thickness);

    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};