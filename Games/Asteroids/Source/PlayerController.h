#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <SpatialComponent.h>
#include <Entity.h>
#include <Systems.h>

#include "AsteroidPhysicsSystem.h"

struct PlayerComponent : public IComponent
{
    float thrust{ 160.f };
	float rotateSpeed{ 5.0f };
	float dampening{ 0.f };
	float respawnTimer{ 0.0f };
	int lives{ 3 };

    REFLECT_DERIVED()
};

struct PlayerController : public ISystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(float deltaTime) override;

private:
    AsteroidPhysics* pRootPhysics;
    PlayerComponent* pPlayerComponent;
};