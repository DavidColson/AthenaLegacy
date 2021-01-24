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
    enum class DrawStyle
    {
        Fill,
        Stroke,
        Both
    };

    enum class DrawSpace
    {
        ForceScreen,
        GameCamera
    };

    enum class GeometryMode
    {
        ZAlign,
        Billboard
    };

    struct Paint
    {   
        DrawStyle drawStyle = DrawStyle::Fill;
        float strokeThickness = 0.0;
        Vec4f strokeColor = Vec4f(1.0f);
        bool strokeLoop = true;
        Vec4f fillColor = Vec4f(1.0f);
        bool billboard = false; // Make this part of instance buffer data
        bool depthTest = true;
    };

    struct PolyshapeMesh
    {
        Mesh strokeMesh;
        Mesh fillMesh;
    };

    void SetDrawSpace(DrawSpace space); // TODO: projection matrix needs to be part of per object data, set it to ortho if space is screen, otherwise use scene camera
    void SetTransform(const Matrixf& transform);
    void SetGeometryMode(GeometryMode mode);
    void SetSortLayer(int16_t sortLayer);

    void Line(const Vec3f& start, const Vec3f& end, const Paint& paint);
    void Circle(const Vec3f& pos, float radius, const Paint& paint);
    void Sector(const Vec3f& pos, float radius, float angleStart, float angleEnd, const Paint& paint);
    void Rect(const Vec3f& center, const Vec2f& size, const Vec4f cornerRad, const Paint& paint);
    void Polyline3D(const eastl::vector<Vec3f>& points, const Paint& paint);
    void Polyshape(const eastl::vector<Vec2f>& points, const Paint& paint);

    PolyshapeMesh CreatePolyshape(const eastl::vector<Vec2f>& points, const Paint& paint);
    void Polyshape(const PolyshapeMesh& shape);


    void Initialize();
    void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
    void OnFrameEnd(Scene& scene, float deltaTime);
};