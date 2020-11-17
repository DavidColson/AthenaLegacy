#include "GfxDraw.h"

#include "GraphicsDevice.h"
#include "GameRenderer.h"
#include "Scene.h"
#include "Mesh.h"
#include "Shader.h"

#include <EASTL/vector.h>

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

    eastl::vector<LineShape> lines;
    eastl::vector<GfxDraw::PolylineShape> polylines;

    Primitive lineMesh;
    AssetHandle lineDrawShader{ AssetHandle("Shaders/LineDraw.hlsl") };
    AssetHandle polyLineDrawShader{ AssetHandle("Shaders/PolylineDraw.hlsl") };

    ConstBufferHandle bufferHandle_perObjectData; 
    ConstBufferHandle bufferHandle_perSceneData; 
    ConstBufferHandle lineShaderInstanceData;
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
            prim.indices.push_back(index1 + 2);
            prim.indices.push_back(index2);
            
            // Second triangle
            prim.indices.push_back(index2);
            prim.indices.push_back(index1 + 2);
            prim.indices.push_back(index2 + 2);
        }
        else if (closed && i == points.size() - 1) // this makes a closed loop of quads
        {
            // First triangle
            prim.indices.push_back(index1);
            prim.indices.push_back(0);
            prim.indices.push_back(index2);

            // Second triangle
            prim.indices.push_back(index2);
            prim.indices.push_back(0);
            prim.indices.push_back(1);
        }
    }
    mesh.primitives.push_back(prim);
}


void GfxDraw::Line(Vec3f start, Vec3f end, Vec4f color, float thickness)
{
    lines.emplace_back(start, end, color, thickness);
}

void GfxDraw::Polyline(const GfxDraw::PolylineShape& shape)
{
    polylines.push_back(shape);
}

void GfxDraw::Initialize()
{
    lines.reserve(16);
    lineMesh = Primitive::NewPlainQuad();

    bufferHandle_perObjectData = GfxDevice::CreateConstantBuffer(sizeof(PerObject), "GfxDraw Per Object data");
    bufferHandle_perSceneData = GfxDevice::CreateConstantBuffer(sizeof(PerScene), "GfxDraw Per Scene data");

    uint32_t bufferSize = sizeof(LineShape) * 16;
	lineShaderInstanceData = GfxDevice::CreateConstantBuffer(bufferSize, "Line Renderer Instance Data");

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
            GfxDevice::SetTopologyType(lineMesh.topologyType);
            GfxDevice::BindVertexBuffers(0, 1, &lineMesh.bufferHandle_vertices);
            GfxDevice::BindIndexBuffer(lineMesh.bufferHandle_indices);

            GfxDevice::DrawIndexedInstanced((int)lineMesh.indices.size(), (int)lines.size(), 0, 0, 0);
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
}

void GfxDraw::OnFrameEnd(Scene& scene, float deltaTime)
{
    lines.clear();
    lines.reserve(16);

    polylines.clear();
}