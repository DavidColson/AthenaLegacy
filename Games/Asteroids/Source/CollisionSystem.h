#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

#include "AsteroidPhysicsSystem.h"

struct PlayerComponent;
struct UpdateContext;

struct CollisionSystem : public IWorldSystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

	void OnBulletAsteroidCollision();
	void OnPlayerAsteroidCollision();

private:

	using PhysicsComponent = eastl::pair<Entity*, AsteroidPhysics*>;
    eastl::vector<PhysicsComponent> asteroids;
    eastl::vector<PhysicsComponent> bullets;

	PhysicsComponent playerPhysics;
	PlayerComponent* pPlayerComponent;
};