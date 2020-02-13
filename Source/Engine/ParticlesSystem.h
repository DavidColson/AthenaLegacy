#pragma once

#include "Maths/Vec2.h"
#include "Maths/Matrix.h"
#include "GraphicsDevice/GraphicsDevice.h"
#include "Log.h"

struct Scene;

#define MAX_PARTICLES_PER_EMITTER 1000

struct Particle
{
	bool bIsAlive{ false };
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
			pNewParticle->bIsAlive = true;
		return pNewParticle;
	}

	void KillParticle(Particle* particle)
	{
		particle->bIsAlive = false;
		deadParticles.push_back(particle);
	}

	Particle pPool[MAX_PARTICLES_PER_EMITTER];
	std::vector<Particle*> deadParticles;
	size_t currentMaxParticleIndex{ 0 };
};

struct CParticleEmitter
{
	bool looping{ true };
	float lifetime{ 1.f };
	int initialCount{ 16 };
	Vec2f initialVelocityMin{ -100.0f, -100.0f};
	Vec2f initialVelocityMax{ 100.0f, 100.0f };
	float initialRotationMin{ 0.f };
	float initialRotationMax{ 0.f };

	ProgramHandle shaderProgram;
  	VertexBufferHandle vertBuffer;
  	ConstBufferHandle transBuffer;
  	VertexBufferHandle instanceBuffer;
	size_t instanceBufferSize{ 0 };

	std::unique_ptr<ParticlePool> particlePool;

	REFLECT()
};

namespace ParticlesSystem
{
	void OnSceneStart(Scene& scene);

	void OnFrame(Scene& scene, float deltaTime);
}