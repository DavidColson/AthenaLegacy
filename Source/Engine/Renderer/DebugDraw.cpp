#include "DebugDraw.h"

#include "Maths/Matrix.h"
#include "Renderer.h"
#include "Profiler.h"

namespace
{
	struct DrawCall
	{
		int vertexCount{ 0 };
		int indexCount{ 0 };
	};
	std::vector<DrawCall> drawQueue;
	
	struct DebugVertex
	{
		Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
		Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	};

	std::vector<DebugVertex> vertBufferData;
	int vertBufferSize = 0;
	std::vector<int> indexBufferData;
	int indexBufferSize = 0;

	ProgramHandle debugShaderProgram;
	VertexBufferHandle vertexBuffer;
	IndexBufferHandle indexBuffer;

	struct TransformData
	{
		Matrixf wvp;
	};

	ConstBufferHandle transformDataBuffer;
}

void DebugDraw::Draw2DCircle(Vec2f pos, float radius, Vec3f color)
{
	float div = 6.282f / 20.f;
	for (int i = 0; i < 20; i++)
	{
		float x = div * (float)i;
		Vec3f point = Vec3f(radius*cos(x), radius*sin(x), 0.0f) + Vec3f(pos.x, pos.y, 0.5f);
		vertBufferData.emplace_back(DebugVertex{ point, color });
		indexBufferData.push_back(i);
	}
	indexBufferData.push_back(0);
	drawQueue.emplace_back(DrawCall{20, 21});
}

void DebugDraw::Draw2DLine(Vec2f start, Vec2f end, Vec3f color)
{
	vertBufferData.emplace_back(DebugVertex{ Vec3f(start.x, start.y, 0.5f), color });
	vertBufferData.emplace_back(DebugVertex{ Vec3f(end.x, end.y, 0.5f), color });
	indexBufferData.push_back(0);
	indexBufferData.push_back(1);

	drawQueue.emplace_back(DrawCall{ 2, 2 });
}

void DebugDraw::Detail::Init()
{
	std::string shaderSrc = "\
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

	std::vector<VertexInputElement> layout;
	layout.push_back({"POSITION",AttributeType::float3});
	layout.push_back({"COLOR", AttributeType::float3});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", layout, "Debug Draw");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Debug Draw");

	debugShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	// Create constant buffer for WVP
	transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Debug draw transforms");
}

void DebugDraw::Detail::DrawQueue()
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing debug");

	if (drawQueue.empty())
		return;

	if (IsValid(vertexBuffer) || vertBufferSize < vertBufferData.size())
	{
		vertBufferSize = (int)vertBufferData.size() + 1000;
		vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(vertBufferSize, sizeof(DebugVertex), "Debug Drawer");
	}

	if (IsValid(indexBuffer) || indexBufferSize < indexBufferData.size())
	{
		indexBufferSize = (int)indexBufferData.size() + 1000;
		indexBuffer = GfxDevice::CreateDynamicIndexBuffer(indexBufferSize, "Debug Drawer");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(vertexBuffer, vertBufferData.data(), vertBufferData.size() * sizeof(DebugVertex));
	GfxDevice::UpdateDynamicIndexBuffer(indexBuffer, indexBufferData.data(), indexBufferData.size() * sizeof(int));

	// Update constant buffer data
	TransformData trans{ Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), 0.1f, 10.0f) };
	GfxDevice::BindConstantBuffer(transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(debugShaderProgram);

	GfxDevice::SetTopologyType(TopologyType::LineStrip);

	GfxDevice::BindVertexBuffer(vertexBuffer);
	GfxDevice::BindIndexBuffer(indexBuffer);

	int vertOffset = 0;
	int indexOffset = 0;
	for (DrawCall& draw : drawQueue)
	{
		GfxDevice::DrawIndexed(draw.indexCount, indexOffset, vertOffset);
		vertOffset += draw.vertexCount;
		indexOffset += draw.indexCount;
	}

	drawQueue.clear();
	vertBufferData.clear();
	indexBufferData.clear();
}
