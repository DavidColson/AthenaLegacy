
#include "renderproxy.h"

#include "Maths/Matrix.h"
#include "Maths/Vec4.h"
#include "Maths/Vec3.h"
#include "Renderer/Renderer.h"

struct TransformData
{
	Matrixf wvp;
	float lineThickness;
	float pad1{ 0.0f };
	float pad2{ 0.0f };
	float pad3{ 0.0f };
};

RenderProxy::RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices, const std::string& name)
{
	vertBuffer = GfxDevice::CreateVertexBuffer(vertices.size(), sizeof(Vertex), vertices.data(), name);
	indexBuffer = GfxDevice::CreateIndexBuffer(indices.size(), indices.data(), name);
	transformBuffer = GfxDevice::CreateConstantBuffer(sizeof(TransformData), name);
}

void RenderProxy::Draw()
{
	// Set vertex buffer as active
	GfxDevice::BindVertexBuffer(vertBuffer);
	GfxDevice::BindIndexBuffer(indexBuffer);

	Matrixf posMat = Matrixf::Translate(pos);
	Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, rot));
	Matrixf scaMat = Matrixf::Scale(sca);
	Matrixf pivotAdjust = Matrixf::Translate(Vec3f(-0.5f, -0.5f, 0.0f));

	Matrixf world = posMat * rotMat * scaMat * pivotAdjust; // transform into world space
	Matrixf view = Matrixf::Translate(Vec3f(0.0f, 0.0f, 0.0f)); // transform into camera space

	Matrixf projection = Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), -1.0f, 10.0f); // transform into screen space
	
	Matrixf wvp = projection * view * world;

	// TODO: Consider using a flag here for picking a shader, so we don't have to do memcpy twice
	TransformData trans{ wvp, lineThickness };
	GfxDevice::BindConstantBuffer(transformBuffer, &trans, ShaderType::Vertex, 0);
	GfxDevice::BindConstantBuffer(transformBuffer, &trans, ShaderType::Geometry, 0);

	GfxDevice::DrawIndexed(GfxDevice::GetIndexBufferSize(indexBuffer), 0, 0);
}
