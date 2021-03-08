#include "AsteroidPhysicsSystem.h"

#include <Rendering/GameRenderer.h>

REFLECT_BEGIN_DERIVED(AsteroidPhysics, SpatialComponent)
REFLECT_MEMBER(velocity)
REFLECT_MEMBER(acceleration)
REFLECT_MEMBER(collisionRadius)
REFLECT_END()

void AsteroidPhysicsSystem::Activate()
{

}

void AsteroidPhysicsSystem::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
		physicsComponents.push_back(static_cast<AsteroidPhysics*>(pComponent));
	}
}

void AsteroidPhysicsSystem::UnregisterComponent(IComponent* pComponent)
{
    eastl::vector<AsteroidPhysics*>::iterator found = eastl::find(physicsComponents.begin(), physicsComponents.end(), pComponent);
	if (found != physicsComponents.end())
	{
		physicsComponents.erase(found);
	}
}

void AsteroidPhysicsSystem::Update(float deltaTime)
{
    for (AsteroidPhysics* pPhysics : physicsComponents)
    {
		pPhysics->velocity = pPhysics->velocity + pPhysics->acceleration * deltaTime;
		pPhysics->SetLocalPosition(pPhysics->GetLocalPosition() + pPhysics->velocity * deltaTime);

        Vec3f localPos = pPhysics->GetLocalPosition();
		if (localPos.x < 0.0f)
		{
			pPhysics->SetLocalPosition(Vec3f(GameRenderer::GetWidth(), localPos.y, localPos.z));
		}
		else if (localPos.x > GameRenderer::GetWidth())
		{
			pPhysics->SetLocalPosition(Vec3f(0.0f, localPos.y, localPos.z));
		}

        localPos = pPhysics->GetLocalPosition();
		if (localPos.y < 0.0f)
		{
			pPhysics->SetLocalPosition(Vec3f(localPos.x, GameRenderer::GetHeight(), localPos.z));
		}
		else if (localPos.y > GameRenderer::GetHeight())
		{
			pPhysics->SetLocalPosition(Vec3f(localPos.x, 0.0f, localPos.z));
		}
    }
}