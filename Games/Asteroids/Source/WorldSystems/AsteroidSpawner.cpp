#include "AsteroidSpawner.h"

#include <Vec3.h>
#include <Profiler.h>
#include <Engine.h>
#include <Rendering/GameRenderer.h>
#include <World.h>
#include <Engine.h>

#include "../Components.h"
#include "../Asteroids.h" 


void AsteroidSpawner::RegisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
    {
        pPlayer = static_cast<PlayerComponent*>(pComponent);
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidSpawnData>())
    {
        pSpawnData = static_cast<AsteroidSpawnData*>(pComponent);
    }
}

void AsteroidSpawner::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
    {
        pPlayer = nullptr;
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidSpawnData>())
    {
        pSpawnData = nullptr;
    }
}

void AsteroidSpawner::Update(UpdateContext& ctx)
{
    PROFILE();

    if (pPlayer->respawnTimer > 0.0f) // Player is dead, stop spawning
        return;

    pSpawnData->timer -= ctx.deltaTime;

    if (pSpawnData->timer <= 0.0f)
    {
        pSpawnData->timer = pSpawnData->timeBetweenSpawns;

        if (pSpawnData->timeBetweenSpawns > 1.0f)
            pSpawnData->timeBetweenSpawns *= pSpawnData->decay;
        else
            pSpawnData->timeBetweenSpawns = 1.0f;
        

        // Actually create an asteroid
        auto randf = []() { return float(rand()) / float(RAND_MAX); };

        // You need to spawn on a window edge
        Vec3f randomLocation;
        Vec3f randomVelocity;
        switch (rand() % 4)
        {
            case 0:
                randomLocation = Vec3f(0.0f, float(rand() % int(GameRenderer::GetHeight())), 0.0f);
                randomVelocity = Vec3f(randf(), randf() * 2.0f - 1.0f, 0.0f); break;
            case 1:
                randomLocation = Vec3f(GameRenderer::GetWidth(), float(rand() % int(GameRenderer::GetHeight())), 0.0f);
                randomVelocity = Vec3f(-randf(), randf() * 2.0f - 1.0f, 0.0f); break;
            case 2:
                randomLocation = Vec3f(float(rand() % int(GameRenderer::GetWidth())), 0.0f, 0.0f);
                randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf(), 0.0f); break;
            case 3:
                randomLocation = Vec3f(float(rand() % int(GameRenderer::GetWidth())), GameRenderer::GetHeight(), 0.0f);
                randomVelocity = Vec3f(randf() * 2.0f - 1.0f, -randf(), 0.0f); break;
            default:
                break;
        }

        Log::Debug("Spawned asteroid");

		Entity* pAsteroid = ctx.pWorld->NewEntity("Asteroid");
		AsteroidComponent* pTemp = pAsteroid->AddNewComponent<AsteroidComponent>();

		AsteroidPhysics* pPhysics = pAsteroid->AddNewComponent<AsteroidPhysics>();
		pPhysics->velocity = randomVelocity * 60.0f;
		pPhysics->SetLocalPosition(randomLocation);
		pPhysics->SetLocalScale(Vec3f(90.0f, 90.0f, 1.0f));
		pPhysics->SetLocalRotation(Vec3f(0.0f, 0.0f, randf() * 6.282f));
		pPhysics->collisionRadius = 40.0f;

		Polyline* pAsteroidPoly = pAsteroid->AddNewComponent<Polyline>(pPhysics->GetId());
		pAsteroidPoly->points = GetRandomAsteroidMesh();
    }
}