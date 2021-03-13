#include "CollisionSystem.h"

#include <Engine.h>
#include <World.h>

#include "PlayerController.h"

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
            asteroids.push_back(PhysicsComponent(pEntity, pPhysics));
            break;
        case CollisionType::Bullet:
            bullets.push_back(PhysicsComponent(pEntity, pPhysics));
            break;
        case CollisionType::Player:
            playerPhysics = PhysicsComponent(pEntity, pPhysics);
            break;
        }
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
	{
        pPlayerComponent = static_cast<PlayerComponent*>(pComponent);
    }
}

void CollisionSystem::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
        AsteroidPhysics* pPhysics = static_cast<AsteroidPhysics*>(pComponent);
        eastl::vector<PhysicsComponent>::iterator found;
        switch (pPhysics->type)
        {
        case CollisionType::Asteroid:
            found = eastl::find(asteroids.begin(), asteroids.end(), PhysicsComponent(pEntity, pPhysics));
            if (found != asteroids.end())
                asteroids.erase(found);
            break;
        case CollisionType::Bullet:
            found = eastl::find(bullets.begin(), bullets.end(), PhysicsComponent(pEntity, pPhysics));
            if (found != bullets.end())
                bullets.erase(found);
            break;
        case CollisionType::Player:
            playerPhysics = PhysicsComponent();
            break;
        }
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
	{
        pPlayerComponent = nullptr;
    }
}

void CollisionSystem::Update(UpdateContext& ctx)
{
    bool bContinueOuter = false;
    for (PhysicsComponent asteroidPair : asteroids)
    {
        float asteroidRad = asteroidPair.second->collisionRadius;
        for (PhysicsComponent bulletPair : bullets)
        {
            float bulletRad = bulletPair.second->collisionRadius;
            
            float distance = (asteroidPair.second->GetLocalPosition() - bulletPair.second->GetLocalPosition()).GetLength();
			float collisionDistance = asteroidRad + bulletRad;

            if (distance < collisionDistance)
			{
				OnBulletAsteroidCollision();
				bContinueOuter = true; break;
			}
        }
        if(bContinueOuter) // The asteroid has been deleted, so skip
            continue;
        
        if (true) // if player is not invincible
        {
            float playerRad = playerPhysics.second->collisionRadius;
            float distance = (asteroidPair.second->GetLocalPosition() - playerPhysics.second->GetLocalPosition()).GetLength();
			float collisionDistance = asteroidRad + playerRad;

            if (distance < collisionDistance)
			{
				OnPlayerAsteroidCollision();
				continue;
			}
        }
    }
}

void CollisionSystem::OnBulletAsteroidCollision()
{
	Log::Debug("Bullet collided with asteroid");
}

void CollisionSystem::OnPlayerAsteroidCollision()
{
	Log::Debug("Player collided with asteroid");
}