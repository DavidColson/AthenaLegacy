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
    struct CBufferPerScene
    {
        Matrixf worldToClipTransform;
        Vec3f camWorldPosition;
        float padding;
        Vec2f screenDimensions;
        float padding1;
        float padding2;
    }; 

    struct CBufferLine
    {
        Matrixf transform;
        Vec4f color;
        Vec3f start;
        float thickness;
        Vec3f end;
        float padding;
    };

    struct CBufferCircle
    {
        Matrixf transform;
        Vec3f location;
        float radius;
        Vec4f color;
        float thickness;
        float angleStart;
        float angleEnd;
        float padding;
    };

    struct CBufferRect
    {
        Matrixf transform;
        Vec3f location;
        float strokeSize;
        Vec4f strokeColor;
        Vec4f fillColor;
        Vec4f cornerRadius;
        Vec2f size;
        float padding;
        float padding2;
    };

    // Instance buffers
    eastl::vector<CBufferLine> lines;
    eastl::vector<CBufferRect> rects;
    eastl::vector<CBufferCircle> circles;

    eastl::vector<eastl::pair<Matrixf, Mesh>> polylines;
    eastl::vector<eastl::pair<Matrixf, Mesh>> polygons;

    Primitive basicQuadMesh;
    AssetHandle lineDrawShader{ AssetHandle("Shaders/LineDraw.hlsl") };
    AssetHandle polyLineDrawShader{ AssetHandle("Shaders/PolylineDraw.hlsl") };
    AssetHandle rectDrawShader{ AssetHandle("Shaders/RectDraw.hlsl") };
    AssetHandle circleDrawShader{ AssetHandle("Shaders/CircleDraw.hlsl") };
    AssetHandle polygonDrawShader{ AssetHandle("Shaders/PolygonDraw.hlsl") };

    ConstBufferHandle bufferHandle_perSceneData; 
    
    ConstBufferHandle transformsInstanceData;
    ConstBufferHandle lineShaderInstanceData;
    ConstBufferHandle rectShaderInstanceData;
    ConstBufferHandle circleShaderInstanceData;
    ConstBufferHandle polylineInstanceData;
    ConstBufferHandle polygonInstanceData;

    BlendStateHandle blendState;

    Matrixf currentTransform = Matrixf::Identity();
}

// TODO: Move geometry gen code somewhere else
void GeneratePolylineMesh(const eastl::vector<Vec3f> points, bool closed, const GfxDraw::Paint& paint, Mesh& outMesh)
{
    Primitive prim;
    prim.vertices.reserve(points.size() * 2);
    prim.uvz0.reserve(points.size() * 2);
    prim.uvz1.reserve(points.size() * 2);
    prim.uvzw0.reserve(points.size() * 2);

    for (size_t i = 0; i < points.size(); i++)
    {
        Vec3f pointPos = points[i];

        prim.vertices.push_back(pointPos); uint16_t index1 = (uint16_t)i * 2;      // inner
        prim.vertices.push_back(pointPos); uint16_t index2 = (uint16_t)i * 2 + 1;  // outer

        prim.colors.push_back(paint.strokeColor);
        prim.colors.push_back(paint.strokeColor);
        
        float endPointUV = 0.0f;
        if (!closed)
        {
            if (i == 0)
                endPointUV = -1.f;
            else if(i == (points.size() - 1))
                endPointUV = 1.0f;
        }

        prim.uvzw0.push_back(Vec4f(endPointUV, -1.0, paint.strokeThickness, float(i))); // inner
        prim.uvzw0.push_back(Vec4f(endPointUV, 1.0, paint.strokeThickness, float(i))); // outer

        // Previous and next points
        if (i == 0)
        {
            if (closed)
                prim.uvz0.push_back(points[points.size() - 1]);
            else
                prim.uvz0.push_back(pointPos * 2.0f - points[1]);
            prim.uvz1.push_back(points[i + 1]);
        }
        else if (i == points.size() - 1)
        {
            prim.uvz0.push_back(points[i - 1]);
            if (closed)
                prim.uvz1.push_back(points[0]);
            else
                prim.uvz1.push_back(pointPos * 2.0f - points[points.size() - 2]);
        }
        else
        {
            prim.uvz0.push_back(points[i - 1]);
            prim.uvz1.push_back(points[i + 1]);
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
    outMesh.primitives.push_back(prim);
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

void GeneratePolygonMesh(const eastl::vector<Vec2f>& points, const GfxDraw::Paint& paint, Mesh& outMesh)
{
    float sumArea = 0.0f;
    for (size_t i = 0; i < points.size(); i++)
    {
        Vec2f a = points[i];
        Vec2f b = points[(i + 1) % points.size()];
        sumArea +=  (b.x - a.x) * (b.y + a.y);
    }
    if (sumArea > 0.0f)
    {
        Log::Crit("Polygon points are not counter clockwise defined");
        return;
    }

    Primitive prim;
    prim.vertices.reserve(points.size());
    for (size_t i = 0; i < points.size(); i++)
    {
        prim.vertices.push_back(Vec3f::Embed2D(points[i]));
        prim.colors.push_back(paint.fillColor);
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

    outMesh.primitives.push_back(prim);
}






void GfxDraw::SetTransform(const Matrixf& transform)
{
    currentTransform = transform;
}

void GfxDraw::Line(const Vec3f& start, const Vec3f& end, const Paint& paint)
{
    lines.emplace_back();
    CBufferLine& newLine = lines.back();
    newLine.transform = currentTransform;
    newLine.start = start;
    newLine.end = end;
    newLine.thickness = paint.strokeThickness;
    newLine.color = paint.strokeColor;
}

void GfxDraw::Circle(const Vec3f& pos, float radius, const Paint& paint)
{
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        circles.emplace_back();
        CBufferCircle& circle = circles.back();
        circle.transform = currentTransform;
        circle.location = pos;
        circle.radius = radius;
        circle.color = paint.fillColor;
    }
    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        circles.emplace_back();
        CBufferCircle& circle = circles.back();
        circle.transform = currentTransform;
        circle.location = pos + Vec3f(0.0f, 0.0f, 0.001f); // Draw in front, maybe better way to do this one day
        circle.radius = radius;
        circle.thickness = paint.strokeThickness;
        circle.color = paint.strokeColor;
    }
}

void GfxDraw::Sector(const Vec3f& pos, float radius, float angleStart, float angleEnd, const Paint& paint)
{
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        circles.emplace_back();
        CBufferCircle& circle = circles.back();
        circle.transform = currentTransform;
        circle.location = pos;
        circle.radius = radius;
        circle.angleStart = angleStart;
        circle.angleEnd = angleEnd;
        circle.color = paint.fillColor;
    }
    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        circles.emplace_back();
        CBufferCircle& circle = circles.back();
        circle.transform = currentTransform;
        circle.location = pos + Vec3f(0.0f, 0.0f, 0.001f); // Draw in front, maybe better way to do this one day
        circle.radius = radius;
        circle.thickness = paint.strokeThickness;
        circle.angleStart = angleStart;
        circle.angleEnd = angleEnd;
        circle.color = paint.strokeColor;
    }
}

void GfxDraw::Rect(const Vec3f& center, const Vec2f& size, const Vec4f cornerRad, const Paint& paint)
{
    rects.emplace_back();
    CBufferRect& rect = rects.back();
    rect.transform = currentTransform;
    rect.cornerRadius = cornerRad;
    rect.location = center;
    rect.size = size;
    
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
        rect.fillColor = paint.fillColor;
    else
        rect.fillColor = Vec4f(0.0f);

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        rect.strokeColor = paint.strokeColor;
        rect.strokeSize = paint.strokeThickness;
    }
}

void GfxDraw::Polyline3D(const eastl::vector<Vec3f>& points, bool closed, const Paint& paint)
{
    polylines.emplace_back();
    eastl::pair<Matrixf, Mesh>& polylineData = polylines.back();
    polylineData.first = currentTransform;
    GeneratePolylineMesh(points, closed, paint, polylineData.second);
}

void GfxDraw::Polyline(const eastl::vector<Vec2f>& points, const Paint& paint)
{
    eastl::vector<Vec3f> points3d; points3d.reserve(points.size());
    for (const Vec2f& vec : points)
    {
        points3d.push_back(Vec3f::Embed2D(vec, 0.0001f));
    }

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        polylines.emplace_back();
        eastl::pair<Matrixf, Mesh>& polylineData = polylines.back();
        polylineData.first = currentTransform;
        GeneratePolylineMesh(points3d, false, paint, polylineData.second);
    }
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        polygons.emplace_back();
        eastl::pair<Matrixf, Mesh>& polygonData = polygons.back();
        polygonData.first = currentTransform;
        GeneratePolygonMesh(points, paint, polygonData.second);
    }
}

void GfxDraw::Polygon(const eastl::vector<Vec2f>& points, const Paint& paint)
{   
    eastl::vector<Vec3f> points3d; points3d.reserve(points.size());
    for (const Vec2f& vec : points)
    {
        points3d.push_back(Vec3f::Embed2D(vec, 0.0001f));
    }

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        polylines.emplace_back();
        eastl::pair<Matrixf, Mesh>& polylineData = polylines.back();
        polylineData.first = currentTransform;
        GeneratePolylineMesh(points3d, true, paint, polylineData.second);
    }
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        polygons.emplace_back();
        eastl::pair<Matrixf, Mesh>& polygonData = polygons.back();
        polygonData.first = currentTransform;
        GeneratePolygonMesh(points, paint, polygonData.second);
    }
}






void GfxDraw::Initialize()
{
    lines.reserve(256);
    rects.reserve(256);
    circles.reserve(256);
    basicQuadMesh = Primitive::NewPlainQuad();

    bufferHandle_perSceneData = GfxDevice::CreateConstantBuffer(sizeof(CBufferPerScene), "GfxDraw Per Scene data");

    {
        uint32_t bufferSize = sizeof(CBufferLine) * 256;
        lineShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Line Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(CBufferRect) * 256;
        rectShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Rect Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(CBufferCircle) * 256;
        circleShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Circle Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(Matrixf) * 256;
        polygonInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Polygon Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(Matrixf) * 256;
        polylineInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Polyline Renderer Instance Data");
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
    CBufferPerScene data;
    data.worldToClipTransform = ctx.projection * ctx.view;
    data.camWorldPosition = ctx.camWorldPosition;
    data.screenDimensions = Vec2f(ctx.screenDimensions.x, ctx.screenDimensions.y);
    GfxDevice::BindConstantBuffer(bufferHandle_perSceneData, &data, ShaderType::Vertex, 0);
    
    // Render Lines
    
    if (Shader* pLineShader = AssetDB::GetAsset<Shader>(lineDrawShader))
    {
        if (GfxDevice::IsValid(pLineShader->program))
        {
            GfxDevice::BindConstantBuffer(lineShaderInstanceData, lines.data(), ShaderType::Vertex, 1);
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
            GfxDevice::BindConstantBuffer(rectShaderInstanceData, rects.data(), ShaderType::Vertex, 1);
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
            GfxDevice::BindConstantBuffer(circleShaderInstanceData, circles.data(), ShaderType::Vertex, 1);
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
                const Mesh& polyline = polylines[i].second;
                const Primitive& prim = polyline.primitives[0];

                GfxDevice::BindConstantBuffer(polylineInstanceData, &polylines[i].first, ShaderType::Vertex, 1);
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
                const Mesh& polygon = polygons[i].second;
                if (polygon.primitives.empty()) continue;
                const Primitive& prim = polygon.primitives[0];

                GfxDevice::BindConstantBuffer(polygonInstanceData, &polygons[i].first, ShaderType::Vertex, 1);
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