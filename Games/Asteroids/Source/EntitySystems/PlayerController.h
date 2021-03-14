#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>


class World;
struct UpdateContext;
struct AsteroidPhysics;
struct PlayerComponent;

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