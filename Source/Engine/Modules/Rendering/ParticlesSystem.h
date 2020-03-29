#pragma once

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Log.h"
#include "Scene.h"
#include <EASTL/unique_ptr.h>

#define MAX_PARTICLES_PER_EMITTER 1000

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

struct CParticleEmitter
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

	ProgramHandle shaderProgram;
  	VertexBufferHandle vertBuffer;
  	ConstBufferHandle transBuffer;
  	VertexBufferHandle instanceBuffer;
	size_t instanceBufferSize{ 0 };

	eastl::unique_ptr<ParticlePool> particlePool;

	REFLECT()
};

namespace ParticlesSystem
{
	void OnAddEmitter(Scene& scene, EntityID entity);
	void OnRemoveEmitter(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}