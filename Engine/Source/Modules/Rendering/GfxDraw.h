#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "Mesh.h"

#include <EASTL/vector.h>

struct Scene;
struct FrameContext;

namespace GfxDraw
{
    struct PolylineShape
    {
        void AddPoint(const Vec3f& pos, const Vec4f& color, float thickness);
        
        void GenerateMesh();
        eastl::vector<eastl::pair<Vec4f, Vec4f>> points;
        bool closed{ false };
        Mesh mesh;
    };

    void Line(Vec3f start, Vec3f end, Vec4f color, float thickness);
    void Rect(Vec3f pos, Vec2f size, Vec4f color, float borderThickness, Vec4f borderRadius);
    void Polyline(const GfxDraw::PolylineShape& shape);

    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};