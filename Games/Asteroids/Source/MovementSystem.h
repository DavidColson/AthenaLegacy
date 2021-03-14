#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

#include "Components.h"

struct UpdateContext;

struct MovementSystem : public IWorldSystem
{
    virtual void Activate() override;

    virtual void Deactivate() override {}

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:

	using PhysicsComponent = eastl::pair<Entity*, AsteroidPhysics*>;
    eastl::vector<PhysicsComponent> physicsComponents;
};