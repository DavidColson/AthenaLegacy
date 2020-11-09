#include "ParticlesSystem.h"

#include "Scene.h"
#include "Matrix.h"
#include "Maths.h"
#include "Profiler.h"
#include "Mesh.h"
#include "Vec4.h"
#include "GameRenderer.h"
#include "Shader.h"

REFLECT_COMPONENT_BEGIN(CParticleEmitter)
REFLECT_MEMBER(looping)
REFLECT_MEMBER(lifetime)
REFLECT_MEMBER(initialCount)
REFLECT_MEMBER(initialVelocityMin)
REFLECT_MEMBER(initialVelocityMax)
REFLECT_MEMBER(initialRotationMin)
REFLECT_MEMBER(initialRotationMax)
REFLECT_MEMBER(initialScaleMin)
REFLECT_MEMBER(initialScaleMax)
REFLECT_END()

struct ParticlesTransform
{
	Matrixf vp;
};

// ***********************************************************************

void RestartEmitter(CParticleEmitter& emitter, CTransform& emitterTransform)
{
	// Create initial particles
	for (size_t i = 0; i < emitter.initialCount; i++)
	{
		Particle* pNewParticle = emitter.particlePool->NewParticle();
		pNewParticle->lifeRemaining = emitter.lifetime;

		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		pNewParticle->position = Vec2f::Project3D(emitterTransform.localPos);
		pNewParticle->rotation = LinearMap(randf(), 0.0f, 1.0f, emitter.initialRotationMin, emitter.initialRotationMax);
		pNewParticle->scale = Vec2f(5.0f, 5.0f);

		Vec2f initialVelocity;
		initialVelocity.x = LinearMap(randf(), 0.0f, 1.0f, emitter.initialVelocityMin.x, emitter.initialVelocityMax.x);
		initialVelocity.y = LinearMap(randf(), 0.0f, 1.0f, emitter.initialVelocityMin.y, emitter.initialVelocityMax.y);

		float scale = LinearMap(randf(), 0.0f, 1.0f, emitter.initialScaleMin, emitter.initialScaleMax); 
		pNewParticle->scale = Vec2f(scale);
		pNewParticle->velocity = initialVelocity;
	}
}

// ***********************************************************************

void ParticlesSystem::OnAddEmitter(Scene& scene, EntityID entity)
{
	CParticleEmitter& emitter = *(scene.Get<CParticleEmitter>(entity));

	emitter.transBuffer = GfxDevice::CreateConstantBuffer(sizeof(ParticlesTransform), "Particles Transform Constant Buffer");
	uint32_t bufferSize = sizeof(Matrixf) * 64;
	emitter.instanceDataBuffer = GfxDevice::CreateConstantBuffer(bufferSize, "Particles instance data buffer");

	emitter.particlePool = eastl::make_unique<ParticlePool>();
	RestartEmitter(emitter, *(scene.Get<CTransform>(entity)));
}

// ***********************************************************************

void ParticlesSystem::OnRemoveEmitter(Scene& scene, EntityID entity)
{
	CParticleEmitter& emitter = *(scene.Get<CParticleEmitter>(entity));
	GfxDevice::FreeConstBuffer(emitter.transBuffer);
	GfxDevice::FreeConstBuffer(emitter.instanceDataBuffer);
}

// ***********************************************************************

void ParticlesSystem::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
	PROFILE();

	for (EntityID ent : SceneIterator<CParticleEmitter, CTransform>(scene))
	{
		// Skip if this entity has a visibility component that is false
		if (scene.Has<CVisibility>(ent))
		{
			if (scene.Get<CVisibility>(ent)->visible == false)
				continue;
		}

		CParticleEmitter* pEmitter = scene.Get<CParticleEmitter>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		// TODO: Move particle simulation to another system that can be modified. We're going to move all the rendering code inside the rendering device.
		// Particle lifetime management also stays here. But we want to give the opportunity to write your own particle simulators
		// Simulate particles and update transforms
		// ****************************************
		eastl::vector<Matrixf> particleTransforms;
		particleTransforms.reserve(64);
		for(int i = 0; i < pEmitter->particlePool->currentMaxParticleIndex; i++)
		{
			Particle* pParticle = &(pEmitter->particlePool->pPool[i]);
			if (!pParticle->isAlive)
				continue;

			pParticle->lifeRemaining -= deltaTime;
			if (pParticle->lifeRemaining < 0.0f)
			{
				pEmitter->particlePool->KillParticle(pParticle);
				continue;
			}

			pParticle->position += pParticle->velocity * deltaTime;

			Matrixf posMat = Matrixf::MakeTranslation(Vec3f::Embed2D(pParticle->position));
			Matrixf rotMat = Matrixf::MakeRotation(Vec3f(0.0f, 0.0f, pParticle->rotation));
			Matrixf scaMat = Matrixf::MakeScale(Vec3f::Embed2D(pParticle->scale));
			Matrixf world = posMat * rotMat * scaMat;
			particleTransforms.push_back(world);
		}

		// If a looping emitter, and all particles are dead, reset it
		// **********************************************************
		if (particleTransforms.empty())
		{
			if (pEmitter->looping == true)
				RestartEmitter(*pEmitter, *pTrans);
			else if (pEmitter->destroyEntityOnEnd == true)
				scene.DestroyEntity(ent);

			continue;
		}

		// Render particles
		// ****************

		Matrixf vp = ctx.projection * ctx.view;
		ParticlesTransform trans{ vp };
		GfxDevice::BindConstantBuffer(pEmitter->transBuffer, &trans, ShaderType::Vertex, 0);
		GfxDevice::BindConstantBuffer(pEmitter->instanceDataBuffer, particleTransforms.data(), ShaderType::Vertex, 1);

		Shader* pShader = AssetDB::GetAsset<Shader>(pEmitter->shader);
        GfxDevice::BindProgram(pShader->program);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		// Set vertex buffer as active
		GfxDevice::BindVertexBuffers(0, 1, &pEmitter->quadPrim.gfxVerticesBuffer);
		GfxDevice::BindVertexBuffers(1, 1, &pEmitter->quadPrim.gfxTexcoordsBuffer);
		GfxDevice::BindVertexBuffers(2, 1, &pEmitter->quadPrim.gfxColorsBuffer);
		GfxDevice::DrawInstanced(4, (int)particleTransforms.size(), 0, 0);
	}
}