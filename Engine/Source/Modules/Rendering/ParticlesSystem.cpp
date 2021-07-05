#include "ParticlesSystem.h"

#include "Matrix.h"
#include "Maths.h"
#include "Profiler.h"
#include "Mesh.h"
#include "Vec4.h"
#include "GameRenderer.h"
#include "Shader.h"
#include "Engine.h"
#include "World.h"

REFLECT_BEGIN_DERIVED(ParticleEmitter, SpatialComponent)
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

void RestartEmitter(ParticleEmitter& emitter)
{
	// Create initial particles
	for (size_t i = 0; i < emitter.initialCount; i++)
	{
		Particle* pNewParticle = emitter.particlePool->NewParticle();
		pNewParticle->lifeRemaining = emitter.lifetime;

		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		pNewParticle->position = Vec2f::Project3D(emitter.GetLocalPosition());
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

ParticlesSystem::~ParticlesSystem()
{
	GameRenderer::UnregisterRenderSystemOpaque(this);
}

void ParticlesSystem::Activate()
{
	GameRenderer::RegisterRenderSystemOpaque(this);
}

void ParticlesSystem::Deactivate()
{
	GameRenderer::UnregisterRenderSystemOpaque(this);
}

void ParticlesSystem::RegisterComponent(Entity* pEntity, IComponent* pComponent)
{
	if (pComponent->GetTypeData() == TypeDatabase::Get<ParticleEmitter>())
	{
		ParticleEmitter* pEmitter = static_cast<ParticleEmitter*>(pComponent);
		emitters[pEntity->GetId()] = pEmitter;

		pEmitter->transBuffer = GfxDevice::CreateConstantBuffer(sizeof(ParticlesTransform), "Particles Transform Constant Buffer");
		uint32_t bufferSize = sizeof(Matrixf) * 64;
		pEmitter->instanceDataBuffer = GfxDevice::CreateConstantBuffer(bufferSize, "Particles instance data buffer");

		pEmitter->particlePool = eastl::make_unique<ParticlePool>();
		RestartEmitter(*pEmitter);
	}
}

void ParticlesSystem::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
	if (pComponent->GetTypeData() == TypeDatabase::Get<ParticleEmitter>())
	{
		ParticleEmitter* pEmitter = static_cast<ParticleEmitter*>(pComponent);
		GfxDevice::FreeConstBuffer(pEmitter->transBuffer);
		GfxDevice::FreeConstBuffer(pEmitter->instanceDataBuffer);
		emitters.erase(pEntity->GetId());
	}
}

void ParticlesSystem::Draw(UpdateContext& ctx, FrameContext& frameCtx)
{
	GFX_SCOPED_EVENT("Scene Draw");
	PROFILE();

	for (eastl::pair<Uuid, ParticleEmitter*> emitterPair : emitters)
    {
        ParticleEmitter* pEmitter = emitterPair.second;
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

			pParticle->lifeRemaining -= ctx.deltaTime;
			if (pParticle->lifeRemaining < 0.0f)
			{
				pEmitter->particlePool->KillParticle(pParticle);
				continue;
			}

			pParticle->position += pParticle->velocity * ctx.deltaTime;

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
				RestartEmitter(*pEmitter);
			else if (pEmitter->destroyEntityOnEnd == true)
				ctx.pWorld->DestroyEntity(emitterPair.first);

			continue;
		}

		// Render particles
		// ****************

		Matrixf vp = frameCtx.projection * frameCtx.view;
		ParticlesTransform trans{ vp };
		GfxDevice::BindConstantBuffer(pEmitter->transBuffer, &trans, ShaderType::Vertex, 0);
		GfxDevice::BindConstantBuffer(pEmitter->instanceDataBuffer, particleTransforms.data(), ShaderType::Vertex, 1);

		Shader* pShader = AssetDB::GetAsset<Shader>(pEmitter->shader);
        GfxDevice::BindProgram(pShader->program);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		// Set vertex buffer as active
		GfxDevice::BindVertexBuffers(0, 1, &pEmitter->quadPrim.bufferHandle_vertices);
		GfxDevice::BindVertexBuffers(1, 1, &pEmitter->quadPrim.bufferHandle_uv0);
		GfxDevice::BindVertexBuffers(2, 1, &pEmitter->quadPrim.bufferHandle_colors);
		GfxDevice::DrawInstanced(4, (int)particleTransforms.size(), 0, 0);
	}
}