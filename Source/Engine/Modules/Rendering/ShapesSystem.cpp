#include "ShapesSystem.h"

#include "Profiler.h"
#include "Vec4.h"
#include "Maths.h"
#include "Mesh.h"
#include "RenderSystem.h"

struct DrawCall
{
	int vertexCount{ 0 };
	int indexCount{ 0 };
};

struct TransformData
{
	Matrixf wvp;
	float lineThickness{ 5.0f };
	float pad1{ 0.0f };
	float pad2{ 0.0f };
	float pad3{ 0.0f };
};

struct CShapesSystemState
{
	eastl::vector<DrawCall> drawQueue;
	eastl::vector<Vert_PosCol> vertexList;
	eastl::vector<uint32_t> indexList;

	ProgramHandle shaderProgram;
	VertexBufferHandle vertexBuffer;
	int vertBufferSize = 0;
	IndexBufferHandle indexBuffer;
	int indexBufferSize = 0;

	ConstBufferHandle transformDataBuffer;
};

namespace
{
	CShapesSystemState* pState = nullptr;
}

// ***********************************************************************

void Shapes::DrawPolyLine(Scene& scene, const VertsVector& verts, float thickness, Vec4f color, bool connected)
{  
	// Vector manipulation here is slow, can do better
	// I recommend using a single frame allocator, or some other with a custom container
	
    VertsVector normals;
    for (int i = 0; i < verts.size(); i++)
    {
        Vec2f edge = verts[i] - verts[mod_floor(i + 1, verts.size())];
        normals.push_back(Vec2f( -edge.y, edge.x).GetNormalized());
    }

    int vertCount = 0;
    int loopExtra = connected ? 1 : 0;
    for (int i = 0; i < verts.size() + loopExtra; i++)
    {
        Vec2f previousEdgeNorm = normals[mod_floor(i - 1, normals.size())];
        Vec2f edgeNorm = normals[mod_floor(i, normals.size())];

        // First element of non loop is itself
        if (!connected && (i == 0))
            previousEdgeNorm = edgeNorm;

        // Second element of non loop must not use next edge as it doesn't exist
        if (!connected && i == (verts.size() + loopExtra - 1))
            edgeNorm = previousEdgeNorm;

        Vec2f cornerBisector = Vec2f((previousEdgeNorm.x + edgeNorm.x) / 2.0f, (previousEdgeNorm.y + edgeNorm.y) / 2.0f).GetNormalized();
	    cornerBisector = cornerBisector / Vec2f::Dot(cornerBisector, edgeNorm);

        // New vertices
        float offset = thickness * 0.2f;
        pState->vertexList.push_back(Vert_PosCol { Vec3f::Embed2D(verts[mod_floor(i, verts.size())] - cornerBisector * offset), color });
        pState->vertexList.push_back(Vert_PosCol { Vec3f::Embed2D(verts[mod_floor(i, verts.size())] + cornerBisector * offset), color });
        
        // Indices
        pState->indexList.push_back(vertCount + 0);
        pState->indexList.push_back(vertCount + 1);
        vertCount += 2;
    }
    pState->drawQueue.emplace_back(DrawCall { vertCount, vertCount });
}

// ***********************************************************************

void Shapes::Initialize()
{
	pState = new CShapesSystemState();

	pState->vertexList.get_allocator().set_name("CShapeSystemState/vertexList");
	pState->drawQueue.get_allocator().set_name("CShapeSystemState/drawQueue");
	pState->indexList.get_allocator().set_name("CShapeSystemState/indexList");
	pState->vertexList.reserve(20480);
	pState->drawQueue.reserve(256);
	pState->indexList.reserve(4096);

    eastl::string shaderSrc = "\
	cbuffer cbTransform\
	{\
		float4x4 WVP;\
	};\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float4 Col : COLOR;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR)\
	{\
		VS_OUTPUT output;\
		output.Pos = mul(inPos, WVP);\
		output.Col = inCol;\
		return output;\
	}\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		return input.Col;\
	}";

	eastl::vector<VertexInputElement> layout;
	layout.push_back({"POSITION",AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float4});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", layout, "Shapes");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Shapes");

	pState->shaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);
	pState->transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Shapes transforms");
}

// ***********************************************************************

void Shapes::Destroy()
{
	GfxDevice::FreeProgram(pState->shaderProgram);
	GfxDevice::FreeVertexBuffer(pState->vertexBuffer);
	GfxDevice::FreeIndexBuffer(pState->indexBuffer);
	GfxDevice::FreeConstBuffer(pState->transformDataBuffer);
}

// ***********************************************************************

void Shapes::OnFrame(Scene& scene, float /* deltaTime */)
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing Shapes");
	
	if (pState->drawQueue.empty())
		return;

	if (!GfxDevice::IsValid(pState->vertexBuffer) || pState->vertBufferSize < pState->vertexList.size())
	{
		if (GfxDevice::IsValid(pState->vertexBuffer)) { GfxDevice::FreeVertexBuffer(pState->vertexBuffer); }
		pState->vertBufferSize = (int)pState->vertexList.size() + 1000;
		pState->vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(pState->vertBufferSize, sizeof(Vert_PosCol), "Shapes System");
	}

	if (!GfxDevice::IsValid(pState->indexBuffer) || pState->indexBufferSize < pState->indexList.size())
	{
		if (GfxDevice::IsValid(pState->indexBuffer)) { GfxDevice::FreeIndexBuffer(pState->indexBuffer); }
		pState->indexBufferSize = (int)pState->indexList.size() + 1000;
		pState->indexBuffer = GfxDevice::CreateDynamicIndexBuffer(pState->indexBufferSize, IndexFormat::UInt, "Shapes System");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(pState->vertexBuffer, pState->vertexList.data(), pState->vertexList.size() * sizeof(Vert_PosCol));
	GfxDevice::UpdateDynamicIndexBuffer(pState->indexBuffer, pState->indexList.data(), pState->indexList.size() * sizeof(uint32_t));

	// Update constant buffer data
	TransformData trans{ Matrixf::Orthographic(0.f, RenderSystem::GetWidth(), 0.0f, RenderSystem::GetHeight(), -1.0f, 10.0f), 5.0f };
	GfxDevice::BindConstantBuffer(pState->transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(pState->shaderProgram);

	GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

	GfxDevice::BindVertexBuffers(1, &pState->vertexBuffer);
	GfxDevice::BindIndexBuffer(pState->indexBuffer);

	int vertOffset = 0;
	int indexOffset = 0;
	for (DrawCall& draw : pState->drawQueue)
	{
		GfxDevice::DrawIndexed(draw.indexCount, indexOffset, vertOffset);
		vertOffset += draw.vertexCount;
		indexOffset += draw.indexCount;
	}

	pState->drawQueue.clear();
	pState->vertexList.clear();
	pState->indexList.clear();
}
