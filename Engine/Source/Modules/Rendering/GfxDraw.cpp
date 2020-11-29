#include "GfxDraw.h"

#include "GraphicsDevice.h"
#include "GameRenderer.h"
#include "Scene.h"
#include "Mesh.h"
#include "Shader.h"

#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>

namespace
{
    struct PerObject
    {
        Matrixf worldToClipTransform;
    };

    struct PerScene
    {
        Vec3f camWorldPosition;
        float padding;
        Vec2f screenDimensions;
        float padding1;
        float padding2;
    };

    struct LineShape
    {
        // This structure maps to a structure in shaders in which variables are 16 byte aligned, so we need padding to make it work properly
        Vec4f color;
        Vec3f start;
        float thickness;
        Vec3f end;
        float padding;

        LineShape(Vec3f _start, Vec3f _end, Vec4f _color, float _thickness) : start(_start), end(_end), color(_color), thickness(_thickness) {}
    };

    struct CircleShape
    {
        Vec3f location;
        float radius;
        Vec4f color;
        float thickness;
        float angleStart;
        float angleEnd;
        float padding;

        CircleShape(Vec3f _location, float _radius, Vec4f _color, float _thickness, float _angleStart, float _angleEnd) : location(_location), radius(_radius), color(_color), thickness(_thickness), angleStart(_angleStart), angleEnd(_angleEnd) {}
    };

    struct RectShape
    {
        Vec3f location;
        float strokeSize;
        Vec4f strokeColor;
        Vec4f fillColor;
        Vec4f cornerRadius;
        Vec2f size;
        float padding;
        float padding2;

        RectShape(Vec3f _location, float _strokeSize, Vec4f _strokeColor, Vec4f _fillColor, Vec4f _cornerRadius, Vec2f _size) : location(_location), strokeSize(_strokeSize), strokeColor(_strokeColor), fillColor(_fillColor), cornerRadius(_cornerRadius), size(_size) {}
    };

    // Instance buffers
    eastl::vector<LineShape> lines;
    eastl::vector<RectShape> rects;
    eastl::vector<CircleShape> circles;

    eastl::vector<GfxDraw::PolylineShape> polylines;
    eastl::vector<GfxDraw::PolygonShape> polygons;

    Primitive basicQuadMesh;
    AssetHandle lineDrawShader{ AssetHandle("Shaders/LineDraw.hlsl") };
    AssetHandle polyLineDrawShader{ AssetHandle("Shaders/PolylineDraw.hlsl") };
    AssetHandle rectDrawShader{ AssetHandle("Shaders/RectDraw.hlsl") };
    AssetHandle circleDrawShader{ AssetHandle("Shaders/CircleDraw.hlsl") };
    AssetHandle polygonDrawShader{ AssetHandle("Shaders/PolygonDraw.hlsl") };

    ConstBufferHandle bufferHandle_perObjectData; 
    ConstBufferHandle bufferHandle_perSceneData; 
    ConstBufferHandle lineShaderInstanceData;
    ConstBufferHandle rectShaderInstanceData;
    ConstBufferHandle circleShaderInstanceData;
    BlendStateHandle blendState;
}

void GfxDraw::PolylineShape::AddPoint(const Vec3f& pos, const Vec4f& color, float thickness)
{
    points.emplace_back(Vec4f(pos.x, pos.y, pos.z, thickness), color);
}

void GfxDraw::PolylineShape::GenerateMesh()
{
    Primitive prim;
    prim.vertices.reserve(points.size() * 2);
    prim.uvz0.reserve(points.size() * 2);
    prim.uvz1.reserve(points.size() * 2);
    prim.uvzw0.reserve(points.size() * 2);

    for (size_t i = 0; i < points.size(); i++)
    {
        Vec3f pointPos = Vec3f::Project4D(points[i].first);
        Vec4f pointCol = points[i].second;
        float thickness = points[i].first.w;

        prim.vertices.push_back(pointPos); uint16_t index1 = (uint16_t)i * 2;      // inner
        prim.vertices.push_back(pointPos); uint16_t index2 = (uint16_t)i * 2 + 1;  // outer

        prim.colors.push_back(pointCol);
        prim.colors.push_back(pointCol);
        
        float endPointUV = 0.0f;
        if (!closed)
        {
            if (i == 0)
                endPointUV = -1.f;
            else if(i == (points.size() - 1))
                endPointUV = 1.0f;
        }

        prim.uvzw0.push_back(Vec4f(endPointUV, -1.0, thickness, float(i))); // inner
        prim.uvzw0.push_back(Vec4f(endPointUV, 1.0, thickness, float(i))); // outer

        // Previous and next points
        if (i == 0)
        {
            if (closed)
                prim.uvz0.push_back(Vec3f::Project4D(points[points.size() - 1].first));
            else
                prim.uvz0.push_back(pointPos * 2.0f - Vec3f::Project4D(points[1].first));
            prim.uvz1.push_back(Vec3f::Project4D(points[i + 1].first));
        }
        else if (i == points.size() - 1)
        {
            prim.uvz0.push_back(Vec3f::Project4D(points[i - 1].first));
            if (closed)
                prim.uvz1.push_back(Vec3f::Project4D(points[0].first));
            else
                prim.uvz1.push_back(pointPos * 2.0f - Vec3f::Project4D(points[points.size() - 2].first));
        }
        else
        {
            prim.uvz0.push_back(Vec3f::Project4D(points[i - 1].first));
            prim.uvz1.push_back(Vec3f::Project4D(points[i + 1].first));
        }
        prim.uvz0.push_back(prim.uvz0.back());
        prim.uvz1.push_back(prim.uvz1.back());

        if (i < points.size() - 1)
        {
            // First triangle
            prim.indices.push_back(index1);
            prim.indices.push_back(index2);
            prim.indices.push_back(index1 + 2);
            
            // Second triangle
            prim.indices.push_back(index2);
            prim.indices.push_back(index2 + 2);
            prim.indices.push_back(index1 + 2);
        }
        else if (closed && i == points.size() - 1) // this makes a closed loop of quads
        {
            // First triangle
            prim.indices.push_back(index1);
            prim.indices.push_back(index2);
            prim.indices.push_back(0);

            // Second triangle
            prim.indices.push_back(index2);
            prim.indices.push_back(1);
            prim.indices.push_back(0);
        }
    }
    mesh.primitives.push_back(prim);
}

void GfxDraw::PolygonShape::AddPoint(const Vec2f& pos)
{
    points.emplace_back(Vec2f(pos.x, pos.y));
}

bool TriangleIsCCW(Vec2f a, Vec2f b, Vec2f c)
{
	return Vec2f::Cross(b - a, b - c) < 0.0f;
}

bool PointInsideTriangle(Vec2f p, Vec2f a, Vec2f b, Vec2f c)
{
    if (Vec2f::Cross(b - a, p - a) < 0.0f) return false;
    if (Vec2f::Cross(c - b, p - b) < 0.0f) return false;
    if (Vec2f::Cross(a - c, p - c) < 0.0f) return false;
    return true;
}

void GfxDraw::PolygonShape::GenerateMesh()
{
    // Triangulate?p - c
    Primitive prim;
    prim.vertices.reserve(points.size());
    for (size_t i = 0; i < points.size(); i++)
    {
        prim.vertices.push_back(Vec3f::Embed2D(points[i]));
        prim.colors.push_back(Vec4f(1.0));
    }

    eastl::vector<int> prev;
    eastl::vector<int> next;

    for (int i = 0; i < (int)points.size(); i++)
    {
        prev.push_back(i - 1);
        next.push_back(i + 1);
    }
    prev[0] = (int)points.size() - 1;
    next[points.size() - 1] = 0;

    size_t i = 0;
    size_t n = points.size();
    int infiniteLoopCounter = 10000;
    while (n > 3)
    {
        infiniteLoopCounter--;
        bool isEar = true;
        if (TriangleIsCCW(points[prev[i]], points[i], points[next[i]]))
        {
            int k = next[next[i]]; // We're going to test every vertex other than the three that define this triangle
            do
            {
                if (PointInsideTriangle(points[k], points[prev[i]], points[i], points[next[i]]))
                {
                    isEar = false;
                    break;
                }
                k = next[k];
            } while (k != prev[i]);
        }
        else
        {
            isEar = false;
        }

        if (isEar)
        {
            prim.indices.push_back(prev[i]);
            prim.indices.push_back((uint16_t)i);
            prim.indices.push_back(next[i]);

            next[prev[i]] = next[i];
            prev[next[i]] = prev[i];
            n--;
            i = prev[i];
        }
        else
        {
            i = next[i];
        }
    }
    // Final triangle
    prim.indices.push_back(prev[i]);
    prim.indices.push_back((uint16_t)i);
    prim.indices.push_back(next[i]);

    mesh.primitives.push_back(prim);
}

void GfxDraw::Line(Vec3f start, Vec3f end, Vec4f color, float thickness)
{
    lines.emplace_back(start, end, color, thickness);
}

void GfxDraw::Rect(Vec3f pos, Vec2f size, Vec4f fillcolor, Vec4f cornerRadius, float strokeThickness, Vec4f strokeColor)
{
    rects.emplace_back(pos, strokeThickness, strokeColor, fillcolor, cornerRadius, size);
}

void GfxDraw::Polyline(const GfxDraw::PolylineShape& shape)
{
    polylines.push_back(shape);
}

void GfxDraw::Polygon(const GfxDraw::PolygonShape& shape)
{
    polygons.push_back(shape);
}

void GfxDraw::Circle(Vec3f pos, float radius, Vec4f color)
{
    circles.emplace_back(pos, radius, color, 0.0f, 0.0f, 0.0f);
}

void GfxDraw::Ring(Vec3f pos, float radius, float thickness, Vec4f color)
{
    circles.emplace_back(pos, radius, color, thickness, 0.0f, 0.0f);
}

void GfxDraw::Pie(Vec3f pos, float radius, float angleStart, float angleEnd, Vec4f color)
{
    circles.emplace_back(pos, radius, color, 0.0f, angleStart, angleEnd);
}

void GfxDraw::Arc(Vec3f pos, float radius, float thickness, float angleStart, float angleEnd, Vec4f color)
{
    circles.emplace_back(pos, radius, color, thickness, angleStart, angleEnd);
}

void GfxDraw::Initialize()
{
    lines.reserve(256);
    rects.reserve(256);
    circles.reserve(256);
    basicQuadMesh = Primitive::NewPlainQuad();

    bufferHandle_perObjectData = GfxDevice::CreateConstantBuffer(sizeof(PerObject), "GfxDraw Per Object data");
    bufferHandle_perSceneData = GfxDevice::CreateConstantBuffer(sizeof(PerScene), "GfxDraw Per Scene data");

    {
        uint32_t bufferSize = sizeof(LineShape) * 256;
        lineShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Line Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(RectShape) * 256;
        rectShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Rect Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(CircleShape) * 256;
        circleShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Circle Renderer Instance Data");
    }

    BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	blender.sourceAlpha = Blend::InverseSrcAlpha;
	blender.destinationAlpha = Blend::One;
	blendState = GfxDevice::CreateBlendState(blender);
}

void GfxDraw::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
	GfxDevice::SetBlending(blendState);
    
    ctx.view.GetForwardVector();
    PerScene data{ctx.camWorldPosition, 0.0f, Vec2f(ctx.screenDimensions.x, ctx.screenDimensions.y)};
    GfxDevice::BindConstantBuffer(bufferHandle_perSceneData, &data, ShaderType::Vertex, 0);
    
    Matrixf worldToClipTransform = ctx.projection * ctx.view;
    PerObject trans{ worldToClipTransform };
    GfxDevice::BindConstantBuffer(bufferHandle_perObjectData, &trans, ShaderType::Vertex, 1);
    
    // TODO: need to render in order, rendering things further away first so that they blend properly.

    // Render Lines
    
    if (Shader* pLineShader = AssetDB::GetAsset<Shader>(lineDrawShader))
    {
        if (GfxDevice::IsValid(pLineShader->program))
        {
            GfxDevice::BindConstantBuffer(lineShaderInstanceData, lines.data(), ShaderType::Vertex, 2);
            GfxDevice::BindProgram(pLineShader->program);
            GfxDevice::SetTopologyType(basicQuadMesh.topologyType);
            GfxDevice::BindVertexBuffers(0, 1, &basicQuadMesh.bufferHandle_vertices);
            GfxDevice::BindIndexBuffer(basicQuadMesh.bufferHandle_indices);

            GfxDevice::DrawIndexedInstanced((int)basicQuadMesh.indices.size(), (int)lines.size(), 0, 0, 0);
        }
    }

    // Render Rects

    if (Shader* pRectShader = AssetDB::GetAsset<Shader>(rectDrawShader))
    {
        if (GfxDevice::IsValid(pRectShader->program))
        {
            GfxDevice::BindConstantBuffer(rectShaderInstanceData, rects.data(), ShaderType::Vertex, 2);
            GfxDevice::BindProgram(pRectShader->program);
            GfxDevice::SetTopologyType(basicQuadMesh.topologyType);
            GfxDevice::BindVertexBuffers(0, 1, &basicQuadMesh.bufferHandle_vertices);
            GfxDevice::BindIndexBuffer(basicQuadMesh.bufferHandle_indices);

            GfxDevice::DrawIndexedInstanced((int)basicQuadMesh.indices.size(), (int)rects.size(), 0, 0, 0);
        }
    }

    // Render Circles

    if (Shader* pCircleShader = AssetDB::GetAsset<Shader>(circleDrawShader))
    {
        if (GfxDevice::IsValid(pCircleShader->program))
        {
            GfxDevice::BindConstantBuffer(circleShaderInstanceData, circles.data(), ShaderType::Vertex, 2);
            GfxDevice::BindProgram(pCircleShader->program);
            GfxDevice::SetTopologyType(basicQuadMesh.topologyType);
            GfxDevice::BindVertexBuffers(0, 1, &basicQuadMesh.bufferHandle_vertices);
            GfxDevice::BindIndexBuffer(basicQuadMesh.bufferHandle_indices);

            GfxDevice::DrawIndexedInstanced((int)basicQuadMesh.indices.size(), (int)circles.size(), 0, 0, 0);
        }
    }

    // Render polylines

    if (Shader* pPolyLineShader = AssetDB::GetAsset<Shader>(polyLineDrawShader))
    {
        if (GfxDevice::IsValid(pPolyLineShader->program))
        {
            GfxDevice::BindProgram(pPolyLineShader->program);
            GfxDevice::SetTopologyType(TopologyType::TriangleList);
            for (size_t i = 0; i < polylines.size(); i++)
            {
                const PolylineShape& polyline = polylines[i];
                const Primitive& prim = polyline.mesh.primitives[0];

                GfxDevice::BindVertexBuffers(0, 1, &prim.bufferHandle_vertices);
                GfxDevice::BindVertexBuffers(1, 1, &prim.bufferHandle_uvzw0);
                GfxDevice::BindVertexBuffers(2, 1, &prim.bufferHandle_uvz0);
                GfxDevice::BindVertexBuffers(3, 1, &prim.bufferHandle_uvz1);
                GfxDevice::BindVertexBuffers(4, 1, &prim.bufferHandle_colors);
                GfxDevice::BindIndexBuffer(prim.bufferHandle_indices);
                
                GfxDevice::DrawIndexed((int)prim.indices.size(), 0, 0);
            }
        }
    }

    // Render polygons

    if (Shader* pPolygonShader = AssetDB::GetAsset<Shader>(polygonDrawShader))
    {
        if (GfxDevice::IsValid(pPolygonShader->program))
        {
            GfxDevice::BindProgram(pPolygonShader->program);
            GfxDevice::SetTopologyType(TopologyType::TriangleList);
            for (size_t i = 0; i < polygons.size(); i++)
            {
                const PolygonShape& polygon = polygons[i];
                const Primitive& prim = polygon.mesh.primitives[0];

                GfxDevice::BindVertexBuffers(0, 1, &prim.bufferHandle_vertices);
                GfxDevice::BindVertexBuffers(1, 1, &prim.bufferHandle_colors);
                GfxDevice::BindIndexBuffer(prim.bufferHandle_indices);
                
                GfxDevice::DrawIndexed((int)prim.indices.size(), 0, 0);
            }
        }
    }
}

void GfxDraw::OnFrameEnd(Scene& scene, float deltaTime)
{
    lines.clear();
    lines.reserve(256);

    rects.clear();
    rects.reserve(256);

    circles.clear();
    circles.reserve(256);

    polylines.clear();
    polygons.clear();
}