#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

struct UpdateContext;

struct AsteroidPhysics : public SpatialComponent
{
	Vec3f velocity;
	Vec3f acceleration;

    float collisionRadius{ 1.0f };

	bool wrapAtEdge{ true };
	
	REFLECT_DERIVED()
};

struct AsteroidPhysicsSystem : public IWorldSystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:

	using PhysicsComponent = eastl::pair<Entity*, AsteroidPhysics*>;
    eastl::vector<PhysicsComponent> physicsComponents;
};