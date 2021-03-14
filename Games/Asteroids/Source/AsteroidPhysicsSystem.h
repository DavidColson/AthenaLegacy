#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

struct UpdateContext;

enum class CollisionType
{
	Player,
	Asteroid,
	Bullet
};

struct AsteroidPhysics : public SpatialComponent
{
    AsteroidPhysics() : SpatialComponent() {}

	Vec3f velocity;
	Vec3f acceleration;
    float collisionRadius{ 1.0f };
	bool wrapAtEdge{ true };
	CollisionType type{ CollisionType::Asteroid };
	
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