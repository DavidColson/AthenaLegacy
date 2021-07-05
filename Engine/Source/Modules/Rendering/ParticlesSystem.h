#pragma once

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Log.h"
#include "AssetDatabase.h"
#include "Mesh.h"
#include "SpatialComponent.h"
#include "Systems.h"

#include <EASTL/shared_ptr.h>

#define MAX_PARTICLES_PER_EMITTER 1000

struct FrameContext;

struct Particle
{
	bool isAlive{ false };
	float lifeRemaining{ 0.0f };

	Vec2f position{ 0.0f, 0.0f };
	Vec2f scale{ 0.0f, 0.0f };
	float rotation{ 0.0f };
	Vec2f velocity{ 0.0f, 0.0f };
};

struct ParticlePool
{
	Particle* NewParticle()
	{
		Particle* pNewParticle = nullptr;
		if (!deadParticles.empty())
		{
			pNewParticle = deadParticles.back();
			deadParticles.pop_back(); 
		}
		else
		{
			if (currentMaxParticleIndex < MAX_PARTICLES_PER_EMITTER)
			{
				currentMaxParticleIndex++;
				pNewParticle = &pPool[currentMaxParticleIndex - 1];
			}
		}

		if (pNewParticle) 
			pNewParticle->isAlive = true;
		return pNewParticle;
	}

	void KillParticle(Particle* particle)
	{
		particle->isAlive = false;
		deadParticles.push_back(particle);
	}

	Particle pPool[MAX_PARTICLES_PER_EMITTER];
	eastl::vector<Particle*> deadParticles;
	size_t currentMaxParticleIndex{ 0 };
};

struct ParticleEmitter : public SpatialComponent
{
	bool looping{ false };
	bool destroyEntityOnEnd{ true };
	float lifetime{ 0.7f };
	int initialCount{ 16 };
	Vec2f initialVelocityMin{ -70.0f, -70.0f};
	Vec2f initialVelocityMax{ 70.0f, 70.0f };
	float initialRotationMin{ 0.f };
	float initialRotationMax{ 3.14159f };
	float initialScaleMin{ 2.0f };
	float initialScaleMax{ 4.5f };

	AssetHandle shader{ AssetHandle("Shaders/Particles.hlsl") };
	
  	Primitive quadPrim{ Primitive::NewPlainQuad() };
  	ConstBufferHandle transBuffer;
  	ConstBufferHandle instanceDataBuffer;
	size_t instanceBufferSize{ 0 };

	eastl::shared_ptr<ParticlePool> particlePool;

	REFLECT_DERIVED()
};

struct ParticlesSystem : public IWorldSystem
{
	~ParticlesSystem();

    virtual void Activate() override;

    virtual void Deactivate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) override;

	eastl::map<Uuid, ParticleEmitter*> emitters;
};