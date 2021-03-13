#include "AsteroidPhysicsSystem.h"

#include "Engine.h"
#include <Rendering/GameRenderer.h>

REFLECT_BEGIN_DERIVED(AsteroidPhysics, SpatialComponent)
REFLECT_MEMBER(velocity)
REFLECT_MEMBER(acceleration)
REFLECT_MEMBER(collisionRadius)
REFLECT_END()

void AsteroidPhysicsSystem::Activate()
{

}

void AsteroidPhysicsSystem::RegisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
		physicsComponents.push_back(static_cast<AsteroidPhysics*>(pComponent));
	}
}

void AsteroidPhysicsSystem::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
    eastl::vector<AsteroidPhysics*>::iterator found = eastl::find(physicsComponents.begin(), physicsComponents.end(), pComponent);
	if (found != physicsComponents.end())
	{
		physicsComponents.erase(found);
	}
}

void AsteroidPhysicsSystem::Update(UpdateContext& ctx)
{
    for (AsteroidPhysics* pPhysics : physicsComponents)
    {
		pPhysics->velocity = pPhysics->velocity + pPhysics->acceleration * ctx.deltaTime;
		pPhysics->SetLocalPosition(pPhysics->GetLocalPosition() + pPhysics->velocity * ctx.deltaTime);

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