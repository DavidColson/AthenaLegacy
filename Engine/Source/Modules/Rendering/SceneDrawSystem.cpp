#include "SceneDrawSystem.h"

#include "Scene.h"
#include "TypeSystem.h"
#include "Profiler.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameRenderer.h"
#include "GfxDraw.h"

//#define DEBUG_BOUNDS

struct cbTransformBuf
{
	Matrixf wvp;
};

namespace
{
    ConstBufferHandle g_transformBufferHandle;
}

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

	for (EntityID ent : SceneIterator<CRenderable, CTransform>(scene))
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
			// Debug draw bounds
			#ifdef DEBUG_BOUNDS
			Vec3f worldTrans = pTrans->globalTransform.GetTranslation();
			AABBf worldBounds = TransformAABB(prim.localBounds, pTrans->globalTransform);
			GfxDraw::Paint paint;
			paint.strokeColor = Vec4f(1.0f);
			paint.strokeThickness = 0.01f;

			Vec3f diff = worldBounds.max - worldBounds.min;	
			GfxDraw::Line(worldBounds.min, worldBounds.min + Vec3f(diff.x, 0.0f, 0.0f), paint);
			GfxDraw::Line(worldBounds.min, worldBounds.min + Vec3f(0.0f, diff.y, 0.0f), paint);
			GfxDraw::Line(worldBounds.min, worldBounds.min + Vec3f(0.0f, 0.0f, diff.z), paint);
			GfxDraw::Line(worldBounds.min + Vec3f(diff.x, 0.0f, 0.0f), worldBounds.max - Vec3f(0.0f, 0.0f, diff.z), paint);
			GfxDraw::Line(worldBounds.min + Vec3f(diff.x, 0.0f, 0.0f), worldBounds.max - Vec3f(0.0f, diff.y, 0.0f), paint);
			GfxDraw::Line(worldBounds.min + Vec3f(0.0f, diff.y, 0.0f), worldBounds.max - Vec3f(0.0f, 0.0f, diff.z), paint);

			GfxDraw::Line(worldBounds.max, worldBounds.max - Vec3f(diff.x, 0.0f, 0.0f), paint);
			GfxDraw::Line(worldBounds.max, worldBounds.max - Vec3f(0.0f, diff.y, 0.0f), paint);
			GfxDraw::Line(worldBounds.max, worldBounds.max - Vec3f(0.0f, 0.0f, diff.z), paint);
			GfxDraw::Line(worldBounds.max - Vec3f(diff.x, 0.0f, 0.0f), worldBounds.min + Vec3f(0.0f, 0.0f, diff.z), paint);
			GfxDraw::Line(worldBounds.max - Vec3f(diff.x, 0.0f, 0.0f), worldBounds.min + Vec3f(0.0f, diff.y, 0.0f), paint);
			GfxDraw::Line(worldBounds.max - Vec3f(0.0f, diff.y, 0.0f), worldBounds.min + Vec3f(0.0f, 0.0f, diff.z), paint);
			#endif

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