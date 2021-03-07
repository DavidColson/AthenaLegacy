#include "SceneDrawSystem.h"

#include "Scene.h"
#include "TypeSystem.h"
#include "Profiler.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameRenderer.h"

struct cbTransformBuf
{
	Matrixf wvp;
};

namespace
{
    ConstBufferHandle g_transformBufferHandle;
}

REFLECT_BEGIN_DERIVED(Renderable, SpatialComponent)
REFLECT_MEMBER(shaderHandle)
REFLECT_MEMBER(meshHandle)
REFLECT_END()

void SceneDrawSystem::Activate()
{
	GameRenderer::RegisterRenderSystemOpaque(this);
	g_transformBufferHandle = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "RenderableTransformBuffer");
}

void SceneDrawSystem::RegisterComponent(IComponent* pComponent)
{
	if (pComponent->GetTypeData() == TypeDatabase::Get<Renderable>())
	{
		renderableComponents.push_back(static_cast<Renderable*>(pComponent));
	}
}

void SceneDrawSystem::UnregisterComponent(IComponent* pComponent)
{
	eastl::vector<Renderable*>::iterator found = eastl::find(renderableComponents.begin(), renderableComponents.end(), pComponent);
	if (found != renderableComponents.end())
	{
		renderableComponents.erase(found);
	}
}
void SceneDrawSystem::Draw(float deltaTime, FrameContext& ctx)
{
	GFX_SCOPED_EVENT("Scene Draw");
	PROFILE();

	for (Renderable* pRenderable : renderableComponents)
	{
		Shader* pShader = AssetDB::GetAsset<Shader>(pRenderable->shaderHandle);
		Mesh* pMesh = AssetDB::GetAsset<Mesh>(pRenderable->meshHandle);
		if (pShader == nullptr || pMesh == nullptr)
			return;

		Matrixf wvp = ctx.projection * ctx.view * pRenderable->GetWorldTransform();
		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(g_transformBufferHandle, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pShader->program);

		for (Primitive& prim : pMesh->primitives)
		{
			GfxDevice::SetTopologyType(prim.topologyType);
			GfxDevice::BindVertexBuffers(0, 1, &prim.bufferHandle_vertices);
			GfxDevice::BindVertexBuffers(1, 1, &prim.bufferHandle_normals);
			GfxDevice::BindVertexBuffers(2, 1, &prim.bufferHandle_uv0);
			GfxDevice::BindVertexBuffers(3, 1, &prim.bufferHandle_colors);

			GfxDevice::BindIndexBuffer(prim.bufferHandle_indices);
			GfxDevice::DrawIndexed((int)prim.indices.size(), 0, 0);
		}
	}
}
