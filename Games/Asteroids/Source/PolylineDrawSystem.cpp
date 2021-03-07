#include "PolylineDrawSystem.h"

#include "Profiler.h"
#include "Vec4.h"
#include "Maths.h"
#include "Mesh.h"
#include "Rendering/GameRenderer.h"

REFLECT_BEGIN_DERIVED(Polyline, SpatialComponent)
REFLECT_MEMBER(thickness)
REFLECT_MEMBER(connected)
REFLECT_END()

struct TransformData
{
	Matrixf wvp;
	float lineThickness{ 5.0f };
	float pad1{ 0.0f };
	float pad2{ 0.0f };
	float pad3{ 0.0f };
};

// ***********************************************************************

void PolylineDrawSystem::AddPolyLine(const VertsVector& verts, float thickness, Vec4f color, bool connected)
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
        vertexList.push_back(Vec3f::Embed2D(verts[mod_floor(i, verts.size())] - cornerBisector * offset));
        colorsList.push_back(color);

        vertexList.push_back(Vec3f::Embed2D(verts[mod_floor(i, verts.size())] + cornerBisector * offset));
        colorsList.push_back(color);
        
        // Indices
        indexList.push_back(vertCount + 0);
        indexList.push_back(vertCount + 1);
        vertCount += 2;
    }
    drawQueue.emplace_back(DrawCall { vertCount, vertCount });
}

// ***********************************************************************

void PolylineDrawSystem::Initialize()
{
	GameRenderer::RegisterRenderSystemOpaque(this);

	vertexList.get_allocator().set_name("CShapeSystemState/vertexList");
	colorsList.get_allocator().set_name("CShapeSystemState/colorsList");
	drawQueue.get_allocator().set_name("CShapeSystemState/drawQueue");
	indexList.get_allocator().set_name("CShapeSystemState/indexList");
	vertexList.reserve(20480);
	colorsList.reserve(20480);
	drawQueue.reserve(256);
	indexList.reserve(4096);

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

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(shaderSrc, "VSMain", "Shapes");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(shaderSrc, "PSMain", "Shapes");

	shaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);
	transformDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), "Shapes transforms");
}

// ***********************************************************************

void PolylineDrawSystem::RegisterComponent(IComponent* pComponent)
{
	if (pComponent->GetTypeData() == TypeDatabase::Get<Polyline>())
	{
		polylineComponents.push_back(static_cast<Polyline*>(pComponent));
	}
}

// ***********************************************************************

void PolylineDrawSystem::UnregisterComponent(IComponent* pComponent)
{
	eastl::vector<Polyline*>::iterator found = eastl::find(polylineComponents.begin(), polylineComponents.end(), pComponent);
	if (found != polylineComponents.end())
	{
		polylineComponents.erase(found);
	}
}

// ***********************************************************************

PolylineDrawSystem::~PolylineDrawSystem()
{
	GfxDevice::FreeProgram(shaderProgram);
	GfxDevice::FreeVertexBuffer(vertexBuffer);
	GfxDevice::FreeVertexBuffer(colorsBuffer);
	GfxDevice::FreeIndexBuffer(indexBuffer);
	GfxDevice::FreeConstBuffer(transformDataBuffer);
}

// ***********************************************************************

void PolylineDrawSystem::Draw(float deltaTime, FrameContext& ctx)
{
	PROFILE();
	GFX_SCOPED_EVENT("Drawing Shapes");

	for (Polyline* pPolyline : polylineComponents)
	{
		Matrixf pivotAdjust = Matrixf::MakeTranslation(Vec3f(-0.5f, -0.5f, 0.0f));
		Matrixf world = pPolyline->GetWorldTransform() * pivotAdjust;

		VertsVector transformedVerts;
		for (const Vec2f& vert : pPolyline->points)
			transformedVerts.push_back(Vec2f::Project4D(world * Vec4f::Embed2D(vert)));

		AddPolyLine(transformedVerts, pPolyline->thickness, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), pPolyline->connected);
	}
	
	if (drawQueue.empty())
		return;

	if (!GfxDevice::IsValid(vertexBuffer) || vertBufferSize < vertexList.size())
	{
		if (GfxDevice::IsValid(vertexBuffer)) { GfxDevice::FreeVertexBuffer(vertexBuffer); }
		vertBufferSize = (int)vertexList.size() + 1000;
		vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(vertBufferSize, sizeof(Vec3f), "Shapes System");
	}

	if (!GfxDevice::IsValid(colorsBuffer) || colorsBufferSize < colorsList.size())
	{
		if (GfxDevice::IsValid(colorsBuffer)) { GfxDevice::FreeVertexBuffer(colorsBuffer); }
		colorsBufferSize = (int)colorsList.size() + 1000;
		colorsBuffer = GfxDevice::CreateDynamicVertexBuffer(colorsBufferSize, sizeof(Vec2f), "Shapes System");
	}

	if (!GfxDevice::IsValid(indexBuffer) || indexBufferSize < indexList.size())
	{
		if (GfxDevice::IsValid(indexBuffer)) { GfxDevice::FreeIndexBuffer(indexBuffer); }
		indexBufferSize = (int)indexList.size() + 1000;
		indexBuffer = GfxDevice::CreateDynamicIndexBuffer(indexBufferSize, IndexFormat::UInt, "Shapes System");
	}

	// Update vert and index buffer data
	GfxDevice::UpdateDynamicVertexBuffer(vertexBuffer, vertexList.data(), vertexList.size() * sizeof(Vec3f));
	GfxDevice::UpdateDynamicVertexBuffer(colorsBuffer, colorsList.data(), colorsList.size() * sizeof(Vec2f));
	GfxDevice::UpdateDynamicIndexBuffer(indexBuffer, indexList.data(), indexList.size() * sizeof(uint32_t));

	// Update constant buffer data
	TransformData trans{ ctx.projection * ctx.view, 5.0f };
	GfxDevice::BindConstantBuffer(transformDataBuffer, &trans, ShaderType::Vertex, 0);

	// Bind shaders
	GfxDevice::BindProgram(shaderProgram);

	GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

	GfxDevice::BindVertexBuffers(0, 1, &vertexBuffer);
	GfxDevice::BindVertexBuffers(1, 1, &colorsBuffer);
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
	vertexList.clear();
	colorsList.clear();
	indexList.clear();
}