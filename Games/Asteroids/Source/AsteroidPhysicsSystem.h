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

	bool wrapAtEdge{ false };
	
	REFLECT_DERIVED()
};

struct AsteroidPhysicsSystem : public ISystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:

    eastl::vector<AsteroidPhysics*> physicsComponents;
};