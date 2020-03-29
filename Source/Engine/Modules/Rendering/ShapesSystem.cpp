#include "ShapesSystem.h"

#include "Profiler.h"
#include "Vec4.h"
#include "Maths.h"

void Shapes::DrawPolyLine(Scene& scene, const VertsVector& verts, float thickness, Vec3f color, bool connected)
{
	CShapesSystemState& state = *(scene.Get<CShapesSystemState>(ENGINE_SINGLETON));
    
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
        state.vertexList.push_back(ShapeVertex { verts[mod_floor(i, verts.size())] + cornerBisector * offset, color });
        state.vertexList.push_back(ShapeVertex { verts[mod_floor(i, verts.size())] - cornerBisector * offset, color });
        
        // Indices
        state.indexList.push_back(vertCount + 0);
        state.indexList.push_back(vertCount + 1);
        vertCount += 2;
    }
    state.drawQueue.emplace_back(DrawCall { vertCount, vertCount });
}

void Shapes::OnShapesSystemStateAdded(Scene& scene, EntityID entity)
{
	CShapesSystemState& state = *(scene.Get<CShapesSystemState>(entity));

	state.vertexList.get_allocator().set_name("CShapeSystemState/vertexList");
	state.drawQueue.get_allocator().set_name("CShapeSystemState/drawQueue");
	state.indexList.get_allocator().set_name("CShapeSystemState/indexList");
	state.vertexList.reserve(20480);
	state.drawQueue.reserve(256);
	state.indexList.reserve(4096);

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
	layout.push_back({"POSITION",AttributeType::Float2});
	layout.push_back({"COLOR", AttributeType::Float3});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", layout, "Shapes");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Shapes");

	state.shaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);
	state.transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Shapes transforms");
}

void Shapes::OnShapesSystemStateRemoved(Scene& scene, EntityID entity)
{
	CShapesSystemState& state = *(scene.Get<CShapesSystemState>(entity));

	GfxDevice::FreeProgram(state.shaderProgram);
	GfxDevice::FreeVertexBuffer(state.vertexBuffer);
	GfxDevice::FreeIndexBuffer(state.indexBuffer);
	GfxDevice::FreeConstBuffer(state.transformDataBuffer);
}

void Shapes::OnFrame(Scene& scene, float /* deltaTime */)
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing Shapes");

	CShapesSystemState& state = *(scene.Get<CShapesSystemState>(ENGINE_SINGLETON));

	if (state.drawQueue.empty())
		return;

	if (!GfxDevice::IsValid(state.vertexBuffer) || state.vertBufferSize < state.vertexList.size())
	{
		if (GfxDevice::IsValid(state.vertexBuffer)) { GfxDevice::FreeVertexBuffer(state.vertexBuffer); }
		state.vertBufferSize = (int)state.vertexList.size() + 1000;
		state.vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(state.vertBufferSize, sizeof(ShapeVertex), "Shapes System");
	}

	if (!GfxDevice::IsValid(state.indexBuffer) || state.indexBufferSize < state.indexList.size())
	{
		if (GfxDevice::IsValid(state.indexBuffer)) { GfxDevice::FreeIndexBuffer(state.indexBuffer); }
		state.indexBufferSize = (int)state.indexList.size() + 1000;
		state.indexBuffer = GfxDevice::CreateDynamicIndexBuffer(state.indexBufferSize, "Shapes System");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(state.vertexBuffer, state.vertexList.data(), state.vertexList.size() * sizeof(ShapeVertex));
	GfxDevice::UpdateDynamicIndexBuffer(state.indexBuffer, state.indexList.data(), state.indexList.size() * sizeof(int));

	// Update constant buffer data
	TransformData trans{ Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), -1.0f, 10.0f), 5.0f };
	GfxDevice::BindConstantBuffer(state.transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(state.shaderProgram);

	GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

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
