#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

#include "AsteroidPhysicsSystem.h"

class World;
struct UpdateContext;

struct PlayerComponent : public IComponent
{
    PlayerComponent() : IComponent() {}

    float thrust{ 160.f };
	float rotateSpeed{ 5.0f };
	float dampening{ 0.f };
	float respawnTimer{ 0.0f };
	int lives{ 3 };

    REFLECT_DERIVED()
};

struct PlayerController : public IEntitySystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

	void SpawnBullet(World* pWorld);

private:
    AsteroidPhysics* pRootPhysics;
    PlayerComponent* pPlayerComponent;
};