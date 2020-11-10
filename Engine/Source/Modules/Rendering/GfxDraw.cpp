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
        Matrixf wvp;
    };

    struct LineShape
    {
        Vec3f start;
        Vec3f end;
        float thickness;

        LineShape(Vec3f _start, Vec3f _end, float _thickness) : start(_start), end(_end), thickness(_thickness) {}
    };

    eastl::vector<LineShape> lines;

    Primitive lineMesh;
    AssetHandle lineDrawShader{ AssetHandle("Shaders/LineDraw.hlsl") };

    ConstBufferHandle lineShaderPerObject; 
}

void GfxDraw::Line(Vec3f start, Vec3f end, float thickness)
{
    lines.emplace_back(start, end, thickness);
}

void GfxDraw::Initialize()
{
    lineMesh = Primitive::NewPlainQuad();
    lineShaderPerObject = GfxDevice::CreateConstantBuffer(sizeof(PerObject), "Line Renderer Per Object Buffer");
}

void GfxDraw::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
    // Go through list of primitives and render them all
    for (LineShape& line : lines)
    {
		Shader* pShader = AssetDB::GetAsset<Shader>(lineDrawShader);
		if (pShader == nullptr)
			return;

        Matrixf transform = Matrixf::MakeTRS(line.start, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0));

		Matrixf wvp = ctx.projection * ctx.view * transform;
		PerObject trans{ wvp };
		GfxDevice::BindConstantBuffer(lineShaderPerObject, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pShader->program);

        GfxDevice::SetTopologyType(lineMesh.topologyType);
        GfxDevice::BindVertexBuffers(0, 1, &lineMesh.gfxVerticesBuffer);
        GfxDevice::BindVertexBuffers(1, 1, &lineMesh.gfxColorsBuffer);

        GfxDevice::BindIndexBuffer(lineMesh.gfxIndexBuffer);
        GfxDevice::DrawIndexed((int)lineMesh.indices.size(), 0, 0);
    } 
}

void GfxDraw::OnFrameEnd(Scene& scene, float deltaTime)
{
    lines.clear();    
}