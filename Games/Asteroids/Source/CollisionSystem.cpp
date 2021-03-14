#include "CollisionSystem.h"

#include <Engine.h>
#include <World.h>

#include "PlayerController.h"
#include "Components.h"
#include "Asteroids.h"

void CollisionSystem::Activate()
{

}

void CollisionSystem::RegisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
        AsteroidPhysics* pPhysics = static_cast<AsteroidPhysics*>(pComponent);
        switch (pPhysics->type)
        {
        case CollisionType::Asteroid:
            asteroidPhysics[pEntity->GetId()] = pPhysics;
            break;
        case CollisionType::Bullet:
            bullets.push_back(pPhysics);
            break;
        case CollisionType::Player:
            pPlayerPhysics = pPhysics;
            break;
        }
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidComponent>())
    {
        asteroids[pEntity->GetId()] = static_cast<AsteroidComponent*>(pComponent);
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
    {
        pPlayerComponent = static_cast<PlayerComponent*>(pComponent);
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<Score>())
    {
        pScoreComponent = static_cast<Score*>(pComponent);
    }
}

void CollisionSystem::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
        AsteroidPhysics* pPhysics = static_cast<AsteroidPhysics*>(pComponent);
        eastl::vector<AsteroidPhysics*>::iterator found;
        switch (pPhysics->type)
        {
        case CollisionType::Asteroid:
            asteroidPhysics.erase(pEntity->GetId());
            break;
        case CollisionType::Bullet:
            found = eastl::find(bullets.begin(), bullets.end(), pPhysics);
            if (found != bullets.end())
                bullets.erase(found);
            break;
        case CollisionType::Player:
            pPlayerPhysics = nullptr;
            break;
        }
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidComponent>())
    {
        asteroids.erase(pEntity->GetId());
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
    {
        pPlayerComponent = nullptr;
    }
}

void CollisionSystem::Update(UpdateContext& ctx)
{
    bool bContinueOuter = false;
    for (eastl::pair<Uuid, AsteroidPhysics*> asteroid : asteroidPhysics)
    {
        AsteroidPhysics* pAsteroid = asteroid.second;
        float asteroidRad = pAsteroid->collisionRadius;
        for (AsteroidPhysics* pBullet : bullets)
        {
            float bulletRad = pBullet->collisionRadius;
            
            float distance = (pAsteroid->GetLocalPosition() - pBullet->GetLocalPosition()).GetLength();
			float collisionDistance = asteroidRad + bulletRad;

            if (distance < collisionDistance)
			{
                OnBulletAsteroidCollision(*(ctx.pWorld), pBullet->GetEntityId(), asteroid.first);
				bContinueOuter = true; break;
			}
        }
        if(bContinueOuter) // The asteroid has been deleted, so skip
            continue;
        
        if (pPlayerComponent->invicibilityTimer <= 0.0f) // if player is not invincible
        {
            float playerRad = pPlayerPhysics->collisionRadius;
            float distance = (pAsteroid->GetLocalPosition() - pPlayerPhysics->GetLocalPosition()).GetLength();
			float collisionDistance = asteroidRad + playerRad;

            if (distance < collisionDistance)
			{
                OnPlayerAsteroidCollision(*(ctx.pWorld), asteroid.first);
			}
        }
    }
}

void CollisionSystem::OnBulletAsteroidCollision(World& world, Uuid bulletEntity, Uuid asteroidEntity)
{
	Log::Debug("Bullet collided with asteroid");

    AsteroidComponent* pAsteroidComponent = asteroids[asteroidEntity];
    AsteroidPhysics* pAsteroidPhysics = asteroidPhysics[asteroidEntity];

    switch (pAsteroidComponent->hitCount)
    {
        case 0: pScoreComponent->currentScore += 20; break;
        case 1: pScoreComponent->currentScore += 50; break;
        case 2: pScoreComponent->currentScore += 100; break;
        default: break;
    }
    pScoreComponent->update = true;

    // TODO: Spawn particles

    if (pAsteroidComponent->hitCount >= 2)
    {
        world.DestroyEntity(asteroidEntity);
        return;
    }

    for (int i = 0; i < 2; i++)
	{
		auto randf = []() { return float(rand()) / float(RAND_MAX); };
		float randomRotation = randf() * 6.282f;

		Vec3f randomVelocity = pAsteroidPhysics->velocity + Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f) * 80.0f;

		Entity* pNewAsteroid = world.NewEntity("Asteroid");
		AsteroidComponent* pTemp = pNewAsteroid->AddNewComponent<AsteroidComponent>();
        pTemp->hitCount = pAsteroidComponent->hitCount + 1;

		AsteroidPhysics* pPhysics = pNewAsteroid->AddNewComponent<AsteroidPhysics>();
		pPhysics->velocity = randomVelocity;
		pPhysics->SetLocalPosition(pAsteroidPhysics->GetLocalPosition());
		pPhysics->SetLocalScale(pAsteroidPhysics->GetLocalScale() * 0.5f);
		pPhysics->SetLocalRotation(Vec3f(0.0f, 0.0f, randomRotation));
		pPhysics->collisionRadius = 40.0f;

		Polyline* pAsteroidPoly = pNewAsteroid->AddNewComponent<Polyline>(pPhysics->GetId());
		pAsteroidPoly->points = GetRandomAsteroidMesh();
	}

    // 6. Destroy this asteroid and the bullet
    world.DestroyEntity(asteroidEntity);
    world.DestroyEntity(bulletEntity);
}

void CollisionSystem::OnPlayerAsteroidCollision(World& world, Uuid asteroidEntity)
{
	Log::Debug("Player collided with asteroid");

    if (pPlayerComponent)
    {
        pPlayerComponent->hasCollidedWithAsteroid = true;
    }

    world.DestroyEntity(asteroidEntity);
}