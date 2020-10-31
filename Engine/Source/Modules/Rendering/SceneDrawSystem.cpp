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

REFLECT_COMPONENT_BEGIN(CCamera)
REFLECT_MEMBER(fov)
REFLECT_MEMBER(horizontalAngle)
REFLECT_MEMBER(verticalAngle)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(CRenderable)
REFLECT_MEMBER(shaderHandle)
REFLECT_MEMBER(meshHandle)
REFLECT_END()

// ***********************************************************************

void SceneDrawSystem::OnSceneCreate(Scene& scene)
{
	g_transformBufferHandle = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "RenderableTransformBuffer");
}

// ***********************************************************************

void SceneDrawSystem::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
    GFX_SCOPED_EVENT("Scene Draw");
    PROFILE();

	for (EntityID ent : SceneView<CRenderable, CTransform>(scene))
    {
		CRenderable* pRenderable = scene.Get<CRenderable>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		Shader* pShader = AssetDB::GetAsset<Shader>(pRenderable->shaderHandle);
		Mesh* pMesh = AssetDB::GetAsset<Mesh>(pRenderable->meshHandle);
		if (pShader == nullptr || pMesh == nullptr)
			return;

		Matrixf wvp = ctx.projection * ctx.view * pTrans->globalTransform;
		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(g_transformBufferHandle, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pShader->program);

		for (Primitive& prim : pMesh->primitives)
		{
			GfxDevice::SetTopologyType(prim.topologyType);
			GfxDevice::BindVertexBuffers(1, &prim.gfxVertBuffer);
			GfxDevice::BindIndexBuffer(prim.gfxIndexBuffer);
			GfxDevice::DrawIndexed((int)prim.indexBuffer.size(), 0, 0);
		}
	}
}