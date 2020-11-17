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
        void AddPoint(const Vec3f& pos, float thickness);
        
        void GenerateMesh();
        eastl::vector<Vec4f> points;
        bool closed{ false };
        Mesh mesh;
    };

    void Line(Vec3f start, Vec3f end, Vec4f color, float thickness);
    void Polyline(PolylineShape line);

    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};