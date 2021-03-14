#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

struct AsteroidPhysics;
struct AsteroidComponent;
struct PlayerComponent;
struct UpdateContext;
struct Score;

class World;

struct CollisionSystem : public IWorldSystem
{
    virtual void Activate() override;

    virtual void Deactivate() override {}

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

	void OnBulletAsteroidCollision(World& world, Uuid bulletEntity, Uuid asteroidEntity);
	void OnPlayerAsteroidCollision(World& world, Uuid asteroidEntity);

private:
	eastl::map<Uuid, AsteroidPhysics*> asteroidPhysics;
	eastl::map<Uuid, AsteroidComponent*> asteroids;

    eastl::vector<AsteroidPhysics*> bullets;

	AsteroidPhysics* pPlayerPhysics;
	PlayerComponent* pPlayerComponent;

	Score* pScoreComponent;
};