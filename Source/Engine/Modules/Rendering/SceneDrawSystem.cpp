#include "SceneDrawSystem.h"

#include "Scene.h"
#include "TypeSystem.h"
#include "Profiler.h"
#include "Mesh.h"

struct cbTransformBuf
{
	Matrixf wvp;
};

namespace
{
    ConstBufferHandle g_transformBufferHandle;
}

REFLECT_BEGIN(CCamera)
REFLECT_MEMBER(fov)
REFLECT_END()

void SceneDrawSystem::OnSceneCreate(Scene& scene)
{
	g_transformBufferHandle = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "RenderableTransformBuffer");
}

void SceneDrawSystem::OnFrame(Scene& scene, float deltaTime)
{
    GFX_SCOPED_EVENT("Scene Draw");
    PROFILE();

	Matrixf view = Matrixf::Identity();
	Matrixf proj = Matrixf::Perspective(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), 0.1f, 100.0f, 60.0f);
	for (EntityID cams : SceneView<CCamera, CTransform>(scene))
	{
		CCamera* pCam = scene.Get<CCamera>(cams);
		CTransform* pTrans = scene.Get<CTransform>(cams);

		Matrixf translate = Matrixf::MakeTranslation(-pTrans->localPos);
		Quatf rotation = Quatf::MakeFromEuler(pTrans->localRot);
		view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;

		proj = Matrixf::Perspective(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), 0.1f, 100.0f, pCam->fov);
	}

	for (EntityID ent : SceneView<CRenderable, CTransform>(scene))
    {
		CRenderable* pCube = scene.Get<CRenderable>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		Matrixf wvp = proj * view * pTrans->globalTransform;
		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(g_transformBufferHandle, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pCube->program);

		for (Primitive& prim : pCube->pMesh->primitives)
		{
			GfxDevice::SetTopologyType(prim.topologyType);
			GfxDevice::BindVertexBuffers(1, &prim.gfxVertBuffer);
			GfxDevice::BindIndexBuffer(prim.gfxIndexBuffer);
			GfxDevice::DrawIndexed((int)prim.indexBuffer.size(), 0, 0);
		}
	}
}