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

        Vec3f start;
        float padding;
        Vec3f end;
        float thickness;

        LineShape(Vec3f _start, Vec3f _end, float _thickness) : start(_start), end(_end), thickness(_thickness) {}
    };

    eastl::vector<LineShape> lines;

    Primitive lineMesh;
    AssetHandle lineDrawShader{ AssetHandle("Shaders/LineDraw.hlsl") };

    ConstBufferHandle lineShaderPerObjectData; 
    ConstBufferHandle lineShaderPerSceneData; 
    ConstBufferHandle lineShaderInstanceData;
    BlendStateHandle blendState;
}

void GfxDraw::Line(Vec3f start, Vec3f end, float thickness)
{
    lines.emplace_back(start, end, thickness);
}

void GfxDraw::Initialize()
{
    lines.reserve(16);
    lineMesh = Primitive::NewPlainQuad();

    lineShaderPerObjectData = GfxDevice::CreateConstantBuffer(sizeof(PerObject), "Line Renderer Per Object data");
    lineShaderPerSceneData = GfxDevice::CreateConstantBuffer(sizeof(PerScene), "Line Renderer Per Scene data");

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
    // Go through list of primitives and render them all
    Shader* pShader = AssetDB::GetAsset<Shader>(lineDrawShader);
    if (pShader == nullptr)
        return;

	GfxDevice::SetBlending(blendState);

	GfxDevice::SetBlending(blendState);
    ctx.view.GetForwardVector();
    PerScene data{ctx.camWorldPosition, 0.0f, Vec2f(ctx.screenDimensions.x, ctx.screenDimensions.y)};
    GfxDevice::BindConstantBuffer(lineShaderPerSceneData, &data, ShaderType::Vertex, 0);
    
    Matrixf worldToClipTransform = ctx.projection * ctx.view;
    PerObject trans{ worldToClipTransform };
    GfxDevice::BindConstantBuffer(lineShaderPerObjectData, &trans, ShaderType::Vertex, 1);
    
    GfxDevice::BindConstantBuffer(lineShaderInstanceData, lines.data(), ShaderType::Vertex, 2);

    GfxDevice::BindProgram(pShader->program);

    GfxDevice::SetTopologyType(lineMesh.topologyType);
    GfxDevice::BindVertexBuffers(0, 1, &lineMesh.gfxVerticesBuffer);
    GfxDevice::BindVertexBuffers(1, 1, &lineMesh.gfxColorsBuffer);

    GfxDevice::BindIndexBuffer(lineMesh.gfxIndexBuffer);
    GfxDevice::DrawIndexedInstanced((int)lineMesh.indices.size(), (int)lines.size(), 0, 0, 0);
}

void GfxDraw::OnFrameEnd(Scene& scene, float deltaTime)
{
    lines.clear();
    lines.reserve(16);
}