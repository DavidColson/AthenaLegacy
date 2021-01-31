#include "GfxDraw.h"

#include "GraphicsDevice.h"
#include "GameRenderer.h"
#include "Scene.h"
#include "Mesh.h"
#include "Shader.h"
#include "LinearAllocator.h"
#include "Profiler.h"

#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/sort.h>

#define MAX_CMD_PER_FRAME 256

namespace
{
    struct CBufferPerScene
    {
        Matrixf worldToClipTransform;
        Matrixf worldToCameraTransform;
        Matrixf screenSpaceToClipTransform;
        Vec3f camWorldPosition;
        float padding;
        Vec2f screenDimensions;
        float padding1;
        float padding2;
    }; 
    
    // TODO: Consider forcing a high alignment amount in the cbuffer memory to avoid having to declare these padding values
    struct LineData
    {
        Matrixf transform;
        Vec4f color;
        Vec3f start;
        float thickness;
        Vec3f end;
        int isScreenSpace;
        int zAlign;
        float pad1;
        float pad2;
        float pad3;
    };

    struct CircleData
    {
        Matrixf transform;
        Vec3f location;
        float radius;
        Vec4f color;
        float thickness;
        float angleStart;
        float angleEnd;
        int isScreenSpace;
        int zAlign;
        float pad1;
        float pad2;
        float pad3;
    };

    struct RectData
    {
        Matrixf transform;
        Vec3f location;
        float strokeSize;
        Vec4f strokeColor;
        Vec4f fillColor;
        Vec4f cornerRadius;
        Vec2f size;
        int isScreenSpace;
        int zAlign;
    };

    struct PolyshapeData
    {
        Matrixf transform;
        int isScreenSpace;
        int zAlign;
        float padding2;
        float padding3;
    };

    LinearAllocator cbufferMemory;
    LinearAllocator cbufferInstanceGroupMemory;

    struct CBuffer
    {
        ConstBufferHandle gfxBufferHandle;
        void* pData;
        size_t size;
    };

    struct DrawCommand
    {
        typedef uint64_t radix_type;
        
        CBuffer cbuffer;
        eastl::vector<VertexBufferHandle> vertBuffers;
        IndexBufferHandle indexBuffer;
        int nIndices{ 0 };
        TopologyType topology;
        AssetHandle shader;
        Vec3f sortLocation;
        bool enableDepthTest{ true };
        uint64_t mKey{ 0 };
    };

    eastl::vector<DrawCommand> drawCommands;
    DrawCommand drawCommandSortBuffer[MAX_CMD_PER_FRAME];
    eastl::vector<GfxDraw::PolyshapeMesh> polyshapeMeshes;

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
    ConstBufferHandle polyshapeInstanceData;

    BlendStateHandle blendState;
    DepthTestStateHandle depthStateEnabled;
    DepthTestStateHandle depthStateDisabled;

    Matrixf currentTransform = Matrixf::Identity();
    GfxDraw::DrawSpace currentDrawSpace = GfxDraw::DrawSpace::GameCamera;
    GfxDraw::GeometryMode currentGeometryMode = GfxDraw::GeometryMode::ZAlign;
    uint16_t currentSortLayer{ (uint16_t)INT16_MAX };
    bool currentDepthTestState{ true };
}

template<typename T>
T* NewCBuffer(CBuffer& outCBuffer, ConstBufferHandle handle)
{
    outCBuffer.gfxBufferHandle = handle;
    outCBuffer.pData = cbufferMemory.Allocate(sizeof(T), 4);
    outCBuffer.size = sizeof(T);
    return new (outCBuffer.pData) T();
}

uint64_t CreateSortKey(bool screenSpace, uint16_t sortLayer, uint16_t vertBufferId)
{
    uint64_t sortKey = 0;
    sortKey |= (uint64_t)vertBufferId << 32;
    sortKey |= (uint64_t)sortLayer << 47;
    sortKey |= (uint64_t)screenSpace << 63;
    return sortKey;
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





void GfxDraw::SetDrawSpace(DrawSpace space)
{
    currentDrawSpace = space;
}

void GfxDraw::SetTransform(const Matrixf& transform)
{
    currentTransform = transform;
}

void GfxDraw::SetGeometryMode(GeometryMode mode)
{
    currentGeometryMode = mode;
}

void GfxDraw::SetSortLayer(int16_t sortLayer)
{
    currentSortLayer = (uint16_t)INT16_MAX + (uint16_t)sortLayer;
}

void GfxDraw::SetDepthTest(bool enabled)
{
    currentDepthTestState = enabled;
}


void GfxDraw::Line(const Vec3f& start, const Vec3f& end, const Paint& paint)
{
    // Allocate and populate line cbuffer data
    CBuffer lineCBuffer;
    LineData* pNewLine = NewCBuffer<LineData>(lineCBuffer, lineShaderInstanceData);
    pNewLine->transform = currentTransform;
    pNewLine->start = start;
    pNewLine->end = end;
    pNewLine->thickness = paint.strokeThickness;
    pNewLine->color = paint.strokeColor;
    pNewLine->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
    pNewLine->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

    // Create draw command for this line
    drawCommands.emplace_back();
    DrawCommand& draw = drawCommands.back();
    draw.cbuffer = lineCBuffer;
    draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
    draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
    draw.nIndices = (int)basicQuadMesh.indices.size();
    draw.topology = basicQuadMesh.topologyType;
    draw.shader = lineDrawShader;
    draw.enableDepthTest = currentDepthTestState;
    draw.sortLocation = currentTransform * start;
    draw.mKey = CreateSortKey(pNewLine->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
}

void GfxDraw::Circle(const Vec3f& pos, float radius, const Paint& paint)
{

    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        CBuffer circleCBuffer;
        CircleData* pCircle = NewCBuffer<CircleData>(circleCBuffer, circleShaderInstanceData);
        pCircle->transform = currentTransform;
        pCircle->location = pos;
        pCircle->radius = radius;
        pCircle->color = paint.fillColor;
        pCircle->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
        pCircle->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);
    
        drawCommands.emplace_back();
        DrawCommand& draw = drawCommands.back();
        draw.cbuffer = circleCBuffer;
        draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
        draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
        draw.nIndices = (int)basicQuadMesh.indices.size();
        draw.topology = basicQuadMesh.topologyType;
        draw.shader = circleDrawShader;
        draw.sortLocation = currentTransform * pos;
        draw.enableDepthTest = currentDepthTestState;
        draw.mKey = CreateSortKey(pCircle->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
    }
    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {   
        CBuffer circleCBuffer;
        CircleData* pCircle = NewCBuffer<CircleData>(circleCBuffer, circleShaderInstanceData);
        pCircle->transform = currentTransform;
        pCircle->location = pos;
        pCircle->radius = radius;
        pCircle->thickness = paint.strokeThickness;
        pCircle->color = paint.strokeColor;
        pCircle->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
        pCircle->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);
    
        drawCommands.emplace_back();
        DrawCommand& draw = drawCommands.back();
        draw.cbuffer = circleCBuffer;
        draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
        draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
        draw.nIndices = (int)basicQuadMesh.indices.size();
        draw.topology = basicQuadMesh.topologyType;
        draw.shader = circleDrawShader;
        draw.sortLocation = currentTransform * pos;
        draw.enableDepthTest = currentDepthTestState;
        draw.mKey = CreateSortKey(pCircle->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
    }

}

void GfxDraw::Sector(const Vec3f& pos, float radius, float angleStart, float angleEnd, const Paint& paint)
{
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        CBuffer circleCBuffer;
        CircleData* pCircle = NewCBuffer<CircleData>(circleCBuffer, circleShaderInstanceData);
        pCircle->transform = currentTransform;
        pCircle->location = pos;
        pCircle->radius = radius;
        pCircle->angleStart = angleStart;
        pCircle->angleEnd = angleEnd;
        pCircle->color = paint.fillColor;
        pCircle->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
        pCircle->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

        drawCommands.emplace_back();
        DrawCommand& draw = drawCommands.back();
        draw.cbuffer = circleCBuffer;
        draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
        draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
        draw.nIndices = (int)basicQuadMesh.indices.size();
        draw.topology = basicQuadMesh.topologyType;
        draw.shader = circleDrawShader;
        draw.enableDepthTest = currentDepthTestState;
        draw.sortLocation = currentTransform * pos;
        draw.mKey = CreateSortKey(pCircle->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
    }
    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        CBuffer circleCBuffer;
        CircleData* pCircle = NewCBuffer<CircleData>(circleCBuffer, circleShaderInstanceData);
        pCircle->transform = currentTransform;
        pCircle->location = pos;
        pCircle->radius = radius;
        pCircle->thickness = paint.strokeThickness;
        pCircle->angleStart = angleStart;
        pCircle->angleEnd = angleEnd;
        pCircle->color = paint.strokeColor;
        pCircle->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
        pCircle->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

        drawCommands.emplace_back();
        DrawCommand& draw = drawCommands.back();
        draw.cbuffer = circleCBuffer;
        draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
        draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
        draw.nIndices = (int)basicQuadMesh.indices.size();
        draw.topology = basicQuadMesh.topologyType;
        draw.shader = circleDrawShader;
        draw.enableDepthTest = currentDepthTestState;
        draw.sortLocation = currentTransform * pos;
        draw.mKey = CreateSortKey(pCircle->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
    }
}

void GfxDraw::Rect(const Vec3f& center, const Vec2f& size, const Vec4f cornerRad, const Paint& paint)
{
    CBuffer rectCBuffer;
    RectData* pRect = NewCBuffer<RectData>(rectCBuffer, rectShaderInstanceData);
    pRect->transform = currentTransform;
    pRect->isScreenSpace = (int)(currentDrawSpace == DrawSpace::ForceScreen);
    pRect->cornerRadius = cornerRad;
    pRect->location = center;
    pRect->size = size;
    pRect->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);
    
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
        pRect->fillColor = paint.fillColor;
    else
        pRect->fillColor = Vec4f(0.0f);

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        pRect->strokeColor = paint.strokeColor;
        pRect->strokeSize = paint.strokeThickness;
    }

    drawCommands.emplace_back();
    DrawCommand& draw = drawCommands.back();
    draw.cbuffer = rectCBuffer;
    draw.vertBuffers.push_back(basicQuadMesh.bufferHandle_vertices);
    draw.indexBuffer = basicQuadMesh.bufferHandle_indices;
    draw.nIndices = (int)basicQuadMesh.indices.size();
    draw.topology = basicQuadMesh.topologyType;
    draw.shader = rectDrawShader;
    draw.sortLocation = currentTransform * center;
    draw.enableDepthTest = currentDepthTestState;
    draw.mKey = CreateSortKey(pRect->isScreenSpace, currentSortLayer, draw.vertBuffers[0].id);
}

void GfxDraw::Polyline3D(const eastl::vector<Vec3f>& points, const Paint& paint)
{
    CBuffer polyshapeCBuffer;
    PolyshapeData* pPolyshape = NewCBuffer<PolyshapeData>(polyshapeCBuffer, polyshapeInstanceData);
    pPolyshape->transform = currentTransform;
    pPolyshape->isScreenSpace = currentDrawSpace == DrawSpace::ForceScreen;
    pPolyshape->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

    polyshapeMeshes.emplace_back();
    PolyshapeMesh& mesh = polyshapeMeshes.back();
    GeneratePolylineMesh(points, paint.strokeLoop, paint, mesh.strokeMesh);

    Primitive& prim = mesh.strokeMesh.primitives[0];
    drawCommands.emplace_back();
    DrawCommand& drawStroke = drawCommands.back();
    drawStroke.cbuffer = polyshapeCBuffer;
    drawStroke.vertBuffers.push_back(prim.bufferHandle_vertices);
    drawStroke.vertBuffers.push_back(prim.bufferHandle_uvzw0);
    drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz0);
    drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz1);
    drawStroke.vertBuffers.push_back(prim.bufferHandle_colors);
    drawStroke.indexBuffer = prim.bufferHandle_indices;
    drawStroke.nIndices = (int)prim.indices.size();
    drawStroke.topology = TopologyType::TriangleList;
    drawStroke.shader = polyLineDrawShader;
    drawStroke.sortLocation = currentTransform.GetTranslation();
    drawStroke.enableDepthTest = currentDepthTestState;
    drawStroke.mKey = CreateSortKey(pPolyshape->isScreenSpace, currentSortLayer, drawStroke.vertBuffers[0].id);
}   

void GfxDraw::Polyshape(const eastl::vector<Vec2f>& points, const Paint& paint)
{
    polyshapeMeshes.emplace_back();
    PolyshapeMesh& mesh = polyshapeMeshes.back();

    CBuffer polyshapeCBuffer;
    PolyshapeData* pPolyshape = NewCBuffer<PolyshapeData>(polyshapeCBuffer, polyshapeInstanceData);
    pPolyshape->transform = currentTransform;
    pPolyshape->isScreenSpace = currentDrawSpace == DrawSpace::ForceScreen;
    pPolyshape->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        eastl::vector<Vec3f> points3d; points3d.reserve(points.size());
        for (const Vec2f& vec : points)
        {
            points3d.push_back(Vec3f::Embed2D(vec, 0.0f));
        }
        GeneratePolylineMesh(points3d, paint.strokeLoop, paint, mesh.strokeMesh);
        Primitive& prim = mesh.strokeMesh.primitives[0];

        drawCommands.emplace_back();
        DrawCommand& drawStroke = drawCommands.back();
        drawStroke.cbuffer = polyshapeCBuffer;
        drawStroke.vertBuffers.push_back(prim.bufferHandle_vertices);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvzw0);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz0);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz1);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_colors);
        drawStroke.indexBuffer = prim.bufferHandle_indices;
        drawStroke.nIndices = (int)prim.indices.size();
        drawStroke.topology = TopologyType::TriangleList;
        drawStroke.shader = polyLineDrawShader;
        drawStroke.sortLocation = currentTransform.GetTranslation();
        drawStroke.enableDepthTest = currentDepthTestState;
        drawStroke.mKey = CreateSortKey(pPolyshape->isScreenSpace, currentSortLayer, drawStroke.vertBuffers[0].id);
    }
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        GeneratePolygonMesh(points, paint, mesh.fillMesh);
        Primitive& prim = mesh.fillMesh.primitives[0];

        drawCommands.emplace_back();
        DrawCommand& drawFill = drawCommands.back();
        drawFill.cbuffer = polyshapeCBuffer;
        drawFill.vertBuffers.push_back(prim.bufferHandle_vertices);
        drawFill.vertBuffers.push_back(prim.bufferHandle_colors);
        drawFill.indexBuffer = prim.bufferHandle_indices;
        drawFill.nIndices = (int)prim.indices.size();
        drawFill.topology = TopologyType::TriangleList;
        drawFill.shader = polygonDrawShader;
        drawFill.sortLocation = currentTransform.GetTranslation();
        drawFill.enableDepthTest = currentDepthTestState;
        drawFill.mKey = CreateSortKey(pPolyshape->isScreenSpace, currentSortLayer, drawFill.vertBuffers[0].id);
    }
}

GfxDraw::PolyshapeMesh GfxDraw::CreatePolyshape(const eastl::vector<Vec2f>& points, const Paint& paint)
{
    PolyshapeMesh shape;

    if (paint.drawStyle == DrawStyle::Stroke || paint.drawStyle == DrawStyle::Both)
    {
        eastl::vector<Vec3f> points3d; points3d.reserve(points.size());
        for (const Vec2f& vec : points)
            points3d.push_back(Vec3f::Embed2D(vec, 0.0f));

        GeneratePolylineMesh(points3d, paint.strokeLoop, paint, shape.strokeMesh);
    }
    if (paint.drawStyle == DrawStyle::Fill || paint.drawStyle == DrawStyle::Both)
    {
        GeneratePolygonMesh(points, paint, shape.fillMesh);
    }
    return shape;
}

void GfxDraw::Polyshape(const PolyshapeMesh& shape)
{
    CBuffer polyshapeCBuffer;
    PolyshapeData* pPolyshape = NewCBuffer<PolyshapeData>(polyshapeCBuffer, polyshapeInstanceData);
    pPolyshape->transform = currentTransform;
    pPolyshape->isScreenSpace = currentDrawSpace == DrawSpace::ForceScreen;
    pPolyshape->zAlign = (int)(currentGeometryMode == GeometryMode::ZAlign);

    if (shape.strokeMesh.primitives.size() > 0)
    {
        const Primitive& prim = shape.strokeMesh.primitives[0];
        drawCommands.emplace_back();
        DrawCommand& drawStroke = drawCommands.back();
        drawStroke.cbuffer = polyshapeCBuffer;
        drawStroke.vertBuffers.push_back(prim.bufferHandle_vertices);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvzw0);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz0);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_uvz1);
        drawStroke.vertBuffers.push_back(prim.bufferHandle_colors);
        drawStroke.indexBuffer = prim.bufferHandle_indices;
        drawStroke.nIndices = (int)prim.indices.size();
        drawStroke.topology = TopologyType::TriangleList;
        drawStroke.shader = polyLineDrawShader;
        drawStroke.sortLocation = currentTransform.GetTranslation();
        drawStroke.enableDepthTest = currentDepthTestState;
        drawStroke.mKey = CreateSortKey(pPolyshape->isScreenSpace, currentSortLayer, drawStroke.vertBuffers[0].id);
    }

    if (shape.fillMesh.primitives.size() > 0)
    {
        const Primitive& prim = shape.fillMesh.primitives[0];
        drawCommands.emplace_back();
        DrawCommand& drawFill = drawCommands.back();
        drawFill.cbuffer = polyshapeCBuffer;
        drawFill.vertBuffers.push_back(prim.bufferHandle_vertices);
        drawFill.vertBuffers.push_back(prim.bufferHandle_colors);
        drawFill.indexBuffer = prim.bufferHandle_indices;
        drawFill.nIndices = (int)prim.indices.size();
        drawFill.topology = TopologyType::TriangleList;
        drawFill.shader = polygonDrawShader;
        drawFill.sortLocation = currentTransform.GetTranslation();
        drawFill.enableDepthTest = currentDepthTestState;
        drawFill.mKey = CreateSortKey(pPolyshape->isScreenSpace, currentSortLayer, drawFill.vertBuffers[0].id);
    }
}





void GfxDraw::Initialize()
{
    drawCommands.reserve(MAX_CMD_PER_FRAME);
    cbufferInstanceGroupMemory.Init(500000);
    cbufferMemory.Init(500000);
    basicQuadMesh = Primitive::NewPlainQuad();

    bufferHandle_perSceneData = GfxDevice::CreateConstantBuffer(sizeof(CBufferPerScene), "GfxDraw Per Scene data");

    {
        uint32_t bufferSize = sizeof(LineData) * MAX_CMD_PER_FRAME;
        lineShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Line Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(RectData) * MAX_CMD_PER_FRAME;
        rectShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Rect Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(CircleData) * MAX_CMD_PER_FRAME;
        circleShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Circle Renderer Instance Data");
    }
    {
        uint32_t bufferSize = sizeof(PolyshapeData) * MAX_CMD_PER_FRAME;
        polyshapeInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Polyshape Renderer Instance Data");
    }

    BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	blender.sourceAlpha = Blend::InverseSrcAlpha;
	blender.destinationAlpha = Blend::One;
	blendState = GfxDevice::CreateBlendState(blender);

    // This blend state will disable alpha blending without writing alpha to the output image
    // Gives us opaque geometry effectively

    // BlendingInfo blender;
	// blender.enabled = true;
	// blender.source = Blend::One;
	// blender.destination = Blend::Zero;
	// blender.sourceAlpha = Blend::One;
	// blender.destinationAlpha = Blend::One;
	// blendState = GfxDevice::CreateBlendState(blender);

    // TODO: Add depth comparison and write masks params
    DepthTestInfo depthOn;
    depthOn.depthEnabled = true;
    depthOn.stencilEnabled = false;
    depthOn.depthWrite = false;
    depthStateEnabled = GfxDevice::CreateDepthTestState(depthOn);

    DepthTestInfo depthOff;
    depthOff.depthEnabled = false;
    depthOff.stencilEnabled = false;
    depthOff.depthWrite = false;
    depthStateDisabled = GfxDevice::CreateDepthTestState(depthOff);
}

void GfxDraw::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{ 
    PROFILE();

	GfxDevice::SetBlending(blendState);
    GfxDevice::SetDepthTest(depthStateEnabled);

    ctx.view.GetForwardVector();
    CBufferPerScene data;
    data.worldToClipTransform = ctx.projection * ctx.view;
    data.worldToCameraTransform = ctx.view;
    data.screenSpaceToClipTransform = Matrixf::Orthographic(0.f, ctx.screenDimensions.x, 0.0f, ctx.screenDimensions.y, -1.0f, 200.0f);;
    data.camWorldPosition = ctx.camWorldPosition;
    data.screenDimensions = Vec2f(ctx.screenDimensions.x, ctx.screenDimensions.y);
    GfxDevice::BindConstantBuffer(bufferHandle_perSceneData, &data, ShaderType::Vertex, 0);

    // Calculate object depth values

    for (DrawCommand& cmd : drawCommands)
    {
        // TODO: Could optimize this by just calculating z and w component in this multiplication
        Vec4f clipSpace = data.worldToClipTransform * Vec4f::Embed3D(cmd.sortLocation);
        float depth = (clipSpace.z / clipSpace.w) * 0.5f + 0.5f;
        depth = 1.0f - depth;
        uint32_t intDepth = uint32_t(depth * (float)UINT32_MAX);
        cmd.mKey &= ~(uint64_t)0xffffffff; // clears bottom 32 bits from the last render pass which may have written to depth
        cmd.mKey |= intDepth;
    }

    // Sort draw commands

    eastl::radix_sort<DrawCommand*, eastl::Internal::extract_radix_key<DrawCommand>>(drawCommands.begin(), drawCommands.end(), drawCommandSortBuffer);


    // Render Draw Commands

    bool lastDepth = true;
    for (int i = 0; i < drawCommands.size(); i++)
    {   
        DrawCommand& cmd = drawCommands[i];

        // Automatic instancing
        int instanceCount = 1;
        if (i + 2 < drawCommands.size())
        {
            int j = i+1;
            if (drawCommands[j].vertBuffers[0].id == cmd.vertBuffers[0].id)
            {
                void* pCbufferData = cbufferInstanceGroupMemory.Allocate(cmd.cbuffer.size, 4);
                memcpy(pCbufferData, cmd.cbuffer.pData, cmd.cbuffer.size);
            }

            while (j < drawCommands.size() && drawCommands[j].vertBuffers[0].id == drawCommands[j-1].vertBuffers[0].id)
            {
                void* pCbufferData = cbufferInstanceGroupMemory.Allocate(drawCommands[j].cbuffer.size, 4);
                memcpy(pCbufferData, drawCommands[j].cbuffer.pData, drawCommands[j].cbuffer.size);
                j++;
            }

            instanceCount = j - i;
            i = j-1;
        }

        if (cmd.enableDepthTest != lastDepth)
        {
            GfxDevice::SetDepthTest(cmd.enableDepthTest ? depthStateEnabled : depthStateDisabled);
            lastDepth = cmd.enableDepthTest;
        }

        if (instanceCount == 1)
        {
            if (Shader* pShader = AssetDB::GetAsset<Shader>(cmd.shader))
            {
                if (GfxDevice::IsValid(pShader->program))
                {
                    GfxDevice::BindConstantBuffer(cmd.cbuffer.gfxBufferHandle, cmd.cbuffer.pData, ShaderType::Vertex, 1);
                    GfxDevice::BindVertexBuffers(0, cmd.vertBuffers.size(), cmd.vertBuffers.data());
                    GfxDevice::BindProgram(pShader->program);
                    GfxDevice::SetTopologyType(cmd.topology);
                    GfxDevice::BindIndexBuffer(cmd.indexBuffer);

                    GfxDevice::DrawIndexed(cmd.nIndices, 0, 0);
                }
            }
        }
        else
        {
            if (Shader* pShader = AssetDB::GetAsset<Shader>(cmd.shader))
            {
                if (GfxDevice::IsValid(pShader->program))
                {
                    GfxDevice::BindConstantBuffer(cmd.cbuffer.gfxBufferHandle, cbufferInstanceGroupMemory.pData, ShaderType::Vertex, 1);
                    GfxDevice::BindVertexBuffers(0, cmd.vertBuffers.size(), cmd.vertBuffers.data());
                    GfxDevice::BindProgram(pShader->program);
                    GfxDevice::SetTopologyType(cmd.topology);
                    GfxDevice::BindIndexBuffer(cmd.indexBuffer);

                    GfxDevice::DrawIndexedInstanced(cmd.nIndices, instanceCount, 0, 0, 0);
                }
            }
        }

        cbufferInstanceGroupMemory.Clear();
    }


    GfxDevice::SetBlending(INVALID_HANDLE);
    GfxDevice::SetDepthTest(INVALID_HANDLE);
}

void GfxDraw::OnFrameEnd(Scene& scene, float deltaTime)
{
    drawCommands.clear();
    cbufferMemory.Clear();
    polyshapeMeshes.clear();
}