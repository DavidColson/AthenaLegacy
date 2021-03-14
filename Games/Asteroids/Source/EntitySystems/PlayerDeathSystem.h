#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

class World;
struct UpdateContext;
struct PlayerComponent;
struct AsteroidPhysics;
struct Polyline;

struct PlayerDeathSystem : public IEntitySystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:
    PlayerComponent* pPlayerComponent;
    AsteroidPhysics* pPlayerPhysics;

    eastl::map<Uuid, Polyline*> polylineComponents;
};