#include "MovementSystem.h"

#include "Engine.h"
#include <Rendering/GameRenderer.h>
#include "World.h"

void MovementSystem::Activate()
{

}

void MovementSystem::RegisterComponent(Entity* pEntity, IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
		physicsComponents.push_back(PhysicsComponent(pEntity, static_cast<AsteroidPhysics*>(pComponent)));
	}
}

void MovementSystem::UnregisterComponent(Entity* pEntity, IComponent* pComponent)
{
    eastl::vector<PhysicsComponent>::iterator found = eastl::find(physicsComponents.begin(), physicsComponents.end(), PhysicsComponent(pEntity, static_cast<AsteroidPhysics*>(pComponent)));
	if (found != physicsComponents.end())
	{
		physicsComponents.erase(found);
	}
}

void MovementSystem::Update(UpdateContext& ctx)
{
    for (PhysicsComponent compPair : physicsComponents)
    {
		Uuid entityId = compPair.first->GetId();
		AsteroidPhysics* pPhysics = compPair.second;
		pPhysics->velocity = pPhysics->velocity + pPhysics->acceleration * ctx.deltaTime;
		pPhysics->SetLocalPosition(pPhysics->GetLocalPosition() + pPhysics->velocity * ctx.deltaTime);

        Vec3f localPos = pPhysics->GetLocalPosition();
		if (localPos.x < 0.0f)
		{
			if (pPhysics->wrapAtEdge) 
				pPhysics->SetLocalPosition(Vec3f(GameRenderer::GetWidth(), localPos.y, localPos.z));
			else
				ctx.pWorld->DestroyEntity(entityId);
		}
		else if (localPos.x > GameRenderer::GetWidth())
		{
			if (pPhysics->wrapAtEdge)
				pPhysics->SetLocalPosition(Vec3f(0.0f, localPos.y, localPos.z));
			else
				ctx.pWorld->DestroyEntity(entityId);
		}

        localPos = pPhysics->GetLocalPosition();
		if (localPos.y < 0.0f)
		{
			if (pPhysics->wrapAtEdge) 
				pPhysics->SetLocalPosition(Vec3f(localPos.x, GameRenderer::GetHeight(), localPos.z));
			else 
				ctx.pWorld->DestroyEntity(entityId);
		}
		else if (localPos.y > GameRenderer::GetHeight())
		{
			if (pPhysics->wrapAtEdge)
				pPhysics->SetLocalPosition(Vec3f(localPos.x, 0.0f, localPos.z));
			else
				ctx.pWorld->DestroyEntity(entityId);
		}
    }
}