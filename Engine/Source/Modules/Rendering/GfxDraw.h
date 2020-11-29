#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "Mesh.h"
#include "Matrix.h"

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

    struct PolygonShape
    {
        void AddPoint(const Vec2f& pos);
        
        void GenerateMesh();
        eastl::vector<Vec2f> points;
        const Vec4f& color{ Vec4f(1.0f) };
        Mesh mesh; // TODO: Consider this being inside an optional, The draw function will auto-generate the mesh if optional is not set
    };

    enum class DrawStyle
    {
        Fill,
        Stroke,
        Both
    };

    enum class DrawSpace
    {
        Screen,
        World
    };

    struct Paint
    {   
        DrawStyle drawStyle = DrawStyle::Fill;
        float strokeThickness = 0.0;
        Vec4f strokeColor = Vec4f(1.0f);
        Vec4f fillColor = Vec4f(1.0f);
        bool billboard = false; // Make this part of instance buffer data
        bool depthTest = true;
    };

    // TODO: The draw functions need to convert the paint data to cbuffer or instance buffer data.
    // For the different draw styles, we may have to enter more than one entrance to the draw buffers.

    void SetDrawSpace(DrawSpace space); // TODO: projection matrix needs to be part of per object data, set it to ortho if space is screen, otherwise use scene camera
    void SetTransform(const Matrixf& transform = Matrixf::Identity());

    void Circle(const Vec3f& pos, float radius, const Paint& paint);
    void Arc(const Vec3f& pos, float radius, float angleStart, float angleEnd, const Paint& paint);
    void Line(const Vec3f& start, const Vec3f& end, const Paint& paint);
    void Rect(const Vec3f& center, const Vec2f& size, const Vec4f cornerRad, const Paint& paint);
    void Polyline(const PolylineShape& shape, const Paint& paint);
    void Polygon(const PolygonShape& shape, const Paint& paint);

    // Need to make variants of these that take a transform to be applied to it during rendering
    void Line(Vec3f start, Vec3f end, Vec4f color, float thickness);
    void Rect(Vec3f pos, Vec2f size, Vec4f fillcolor, Vec4f cornerRadius = Vec4f(0.0f), float strokeThickness = 0.0f, Vec4f strokeColor = Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    void Polyline(const GfxDraw::PolylineShape& shape);
    void Polygon(const GfxDraw::PolygonShape& shape);
    void Circle(Vec3f pos, float radius, Vec4f color);
    void Ring(Vec3f pos, float radius, float thickness, Vec4f color);
    void Pie(Vec3f pos, float radius, float angleStart, float angleEnd, Vec4f color);
    void Arc(Vec3f pos, float radius, float thickness, float angleStart, float angleEnd, Vec4f color);

    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};