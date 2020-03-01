#include "DebugDraw.h"

#include "Profiler.h"

void DebugDraw::Draw2DCircle(Scene& scene, Vec2f pos, float radius, Vec3f color)
{
	CDebugDrawingState& state = *(scene.Get<CDebugDrawingState>(ENGINE_SINGLETON));

	float div = 6.282f / 20.f;
	for (int i = 0; i < 20; i++)
	{
		float x = div * (float)i;
		Vec3f point = Vec3f(radius*cos(x), radius*sin(x), 0.0f) + Vec3f(pos.x, pos.y, 0.5f);
		state.vertexList.emplace_back(DebugVertex{ point, color });
		state.indexList.push_back(i);
	}
	state.indexList.push_back(0);
	state.drawQueue.emplace_back(DrawCall{20, 21});
}

void DebugDraw::Draw2DLine(Scene& scene, Vec2f start, Vec2f end, Vec3f color)
{
	CDebugDrawingState& state = *(scene.Get<CDebugDrawingState>(ENGINE_SINGLETON));

	state.vertexList.emplace_back(DebugVertex{ Vec3f(start.x, start.y, 0.5f), color });
	state.vertexList.emplace_back(DebugVertex{ Vec3f(end.x, end.y, 0.5f), color });
	state.indexList.push_back(0);
	state.indexList.push_back(1);

	state.drawQueue.emplace_back(DrawCall{ 2, 2 });
}

void DebugDraw::OnDebugDrawStateAdded(Scene& scene, EntityID entity)
{
	CDebugDrawingState& state = *(scene.Get<CDebugDrawingState>(ENGINE_SINGLETON));

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
	layout.push_back({"POSITION",AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float3});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", layout, "Debug Draw");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Debug Draw");

	state.debugShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	// Create constant buffer for WVP
	state.transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Debug draw transforms");
}

void DebugDraw::OnDebugDrawStateRemoved(Scene& scene, EntityID entity)
{
	CDebugDrawingState& state = *(scene.Get<CDebugDrawingState>(ENGINE_SINGLETON));

	GfxDevice::FreeProgram(state.debugShaderProgram);
	GfxDevice::FreeVertexBuffer(state.vertexBuffer);
	GfxDevice::FreeIndexBuffer(state.indexBuffer);
	GfxDevice::FreeConstBuffer(state.transformDataBuffer);
}

void DebugDraw::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing debug");

	CDebugDrawingState& state = *(scene.Get<CDebugDrawingState>(ENGINE_SINGLETON));

	if (state.drawQueue.empty())
		return;

	// TODO: MEMORY LEAK HERE Release the old buffer when you create new one
	if (!GfxDevice::IsValid(state.vertexBuffer) || state.vertBufferSize < state.vertexList.size())
	{
		state.vertBufferSize = (int)state.vertexList.size() + 1000;
		state.vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(state.vertBufferSize, sizeof(DebugVertex), "Debug Drawer");
	}

	if (!GfxDevice::IsValid(state.indexBuffer) || state.indexBufferSize < state.indexList.size())
	{
		state.indexBufferSize = (int)state.indexList.size() + 1000;
		state.indexBuffer = GfxDevice::CreateDynamicIndexBuffer(state.indexBufferSize, "Debug Drawer");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(state.vertexBuffer, state.vertexList.data(), state.vertexList.size() * sizeof(DebugVertex));
	GfxDevice::UpdateDynamicIndexBuffer(state.indexBuffer, state.indexList.data(), state.indexList.size() * sizeof(int));

	// Update constant buffer data
	TransformData trans{ Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), 0.1f, 10.0f) };
	GfxDevice::BindConstantBuffer(state.transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(state.debugShaderProgram);

	GfxDevice::SetTopologyType(TopologyType::LineStrip);

	GfxDevice::BindVertexBuffers(1, &state.vertexBuffer);
	GfxDevice::BindIndexBuffer(state.indexBuffer);

	int vertOffset = 0;
	int indexOffset = 0;
	for (DrawCall& draw : state.drawQueue)
	{
		GfxDevice::DrawIndexed(draw.indexCount, indexOffset, vertOffset);
		vertOffset += draw.vertexCount;
		indexOffset += draw.indexCount;
	}

	state.drawQueue.clear();
	state.vertexList.clear();
	state.indexList.clear();
}
