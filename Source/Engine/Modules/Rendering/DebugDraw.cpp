#include "DebugDraw.h"

#include "Profiler.h"
#include "RenderSystem.h"

struct DrawCall
{
	int vertexCount{ 0 };
	int indexCount{ 0 };
};

struct DebugVertex
{
	Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
};

struct TransformData
{
	Matrixf wvp;
};


struct CDebugDrawingState
{
	eastl::vector<DrawCall> drawQueue;
	eastl::vector<DebugVertex> vertexList;
	eastl::vector<uint32_t> indexList;

	ProgramHandle debugShaderProgram;
	VertexBufferHandle vertexBuffer;
	int vertBufferSize = 0;
	IndexBufferHandle indexBuffer;
	int indexBufferSize = 0;

	ConstBufferHandle transformDataBuffer;
};

namespace
{
	CDebugDrawingState* pState = nullptr;
}

void DebugDraw::Draw2DCircle(Scene& scene, Vec2f pos, float radius, Vec3f color)
{
	float div = 6.282f / 20.f;
	for (int i = 0; i < 20; i++)
	{
		float x = div * (float)i;
		Vec3f point = Vec3f(radius*cosf(x), radius*sinf(x), 0.0f) + Vec3f(pos.x, pos.y, 0.5f);
		pState->vertexList.emplace_back(DebugVertex{ point, color });
		pState->indexList.push_back(i);
	}
	pState->indexList.push_back(0);
	pState->drawQueue.emplace_back(DrawCall{20, 21});
}

void DebugDraw::Draw2DLine(Scene& scene, Vec2f start, Vec2f end, Vec3f color)
{
	pState->vertexList.emplace_back(DebugVertex{ Vec3f(start.x, start.y, 0.5f), color });
	pState->vertexList.emplace_back(DebugVertex{ Vec3f(end.x, end.y, 0.5f), color });
	pState->indexList.push_back(0);
	pState->indexList.push_back(1);

	pState->drawQueue.emplace_back(DrawCall{ 2, 2 });
}

void DebugDraw::Initialize()
{
	pState = new CDebugDrawingState();

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
	layout.push_back({"COLOR", AttributeType::Float3});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", layout, "Debug Draw");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Debug Draw");

	pState->debugShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	// Create constant buffer for WVP
	pState->transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Debug draw transforms");
}

void DebugDraw::Destroy()
{
	GfxDevice::FreeProgram(pState->debugShaderProgram);
	GfxDevice::FreeVertexBuffer(pState->vertexBuffer);
	GfxDevice::FreeIndexBuffer(pState->indexBuffer);
	GfxDevice::FreeConstBuffer(pState->transformDataBuffer);
}

void DebugDraw::OnFrame(Scene& scene, float /* deltaTime */)
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing debug");

	if (pState->drawQueue.empty())
		return;

	// TODO: MEMORY LEAK HERE Release the old buffer when you create new one
	if (!GfxDevice::IsValid(pState->vertexBuffer) || pState->vertBufferSize < pState->vertexList.size())
	{
		if (GfxDevice::IsValid(pState->vertexBuffer)) { GfxDevice::FreeVertexBuffer(pState->vertexBuffer); }
		pState->vertBufferSize = (int)pState->vertexList.size() + 1000;
		pState->vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(pState->vertBufferSize, sizeof(DebugVertex), "Debug Drawer");
	}

	if (!GfxDevice::IsValid(pState->indexBuffer) || pState->indexBufferSize < pState->indexList.size())
	{
		if (GfxDevice::IsValid(pState->indexBuffer)) { GfxDevice::FreeIndexBuffer(pState->indexBuffer); }
		pState->indexBufferSize = (int)pState->indexList.size() + 1000;
		pState->indexBuffer = GfxDevice::CreateDynamicIndexBuffer(pState->indexBufferSize, IndexFormat::UInt, "Debug Drawer");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(pState->vertexBuffer, pState->vertexList.data(), pState->vertexList.size() * sizeof(DebugVertex));
	GfxDevice::UpdateDynamicIndexBuffer(pState->indexBuffer, pState->indexList.data(), pState->indexList.size() * sizeof(uint32_t));

	// Update constant buffer data
	TransformData trans{ Matrixf::Orthographic(0.f, RenderSystem::GetWidth(), 0.0f, RenderSystem::GetHeight(), 0.1f, 10.0f) };
	GfxDevice::BindConstantBuffer(pState->transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(pState->debugShaderProgram);

	GfxDevice::SetTopologyType(TopologyType::LineStrip);

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
