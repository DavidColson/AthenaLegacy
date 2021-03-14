#include "PlayerDeathSystem.h"

#include <World.h>
#include <Engine.h>
#include <Rendering/GameRenderer.h>

#include "Components.h"

void PlayerDeathSystem::Activate()
{

}

void PlayerDeathSystem::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
	{
		pPlayerComponent = static_cast<PlayerComponent*>(pComponent);
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
    {
        pPlayerPhysics = static_cast<AsteroidPhysics*>(pComponent);
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<Polyline>())
    {
        polylineComponents[pComponent->GetId()] = static_cast<Polyline*>(pComponent);
    }
}

void PlayerDeathSystem::UnregisterComponent(IComponent* pComponent)
{
    if (pComponent == pPlayerComponent)
    {
        pPlayerComponent = nullptr;
    }

    if (pComponent == pPlayerPhysics)
    {
        pPlayerPhysics = nullptr;
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<Polyline>())
    {
        polylineComponents.erase(pComponent->GetId());
    }
}

void PlayerDeathSystem::Update(UpdateContext& ctx)
{
    if (pPlayerComponent)
    {
        if (pPlayerComponent->hasCollidedWithAsteroid && pPlayerComponent->invicibilityTimer <= 0.0f)
        {
            pPlayerComponent->hasCollidedWithAsteroid = false;
            pPlayerComponent->respawnTimer = 5.0f; // This being above 0 means the player is dead effectively
            polylineComponents[pPlayerComponent->playerPolylineComponent]->visible = false;

            float w = GameRenderer::GetWidth();
	        float h = GameRenderer::GetHeight();
            pPlayerPhysics->SetLocalPosition(Vec3f(w/2.0f, h/2.0f, 0.0f));
            pPlayerPhysics->SetLocalRotation(Vec3f(0.0f));
            pPlayerPhysics->velocity = Vec3f(0.0f);
            pPlayerPhysics->acceleration = Vec3f(0.0f);

            Uuid lifeId = pPlayerComponent->lives.back();
            pPlayerComponent->lives.erase(pPlayerComponent->lives.begin() + (pPlayerComponent->lives.size() - 1));
            polylineComponents[lifeId]->visible = false;
        }

        if (pPlayerComponent->respawnTimer > 0.0f)
        {
            pPlayerComponent->respawnTimer -= ctx.deltaTime;

            if (pPlayerComponent->respawnTimer <= 0.0f)
            {
                polylineComponents[pPlayerComponent->playerPolylineComponent]->visible = true;
                pPlayerComponent->invicibilityTimer = 5.0f;
            }
        }

        if (pPlayerComponent->invicibilityTimer > 0.0f)
        {
            pPlayerComponent->invicibilityTimer -= ctx.deltaTime;
            if (pPlayerComponent->invicibilityTimer <= 0.0f)
            {
                polylineComponents[pPlayerComponent->playerPolylineComponent]->visible = true;
                return;
            }

            pPlayerComponent->flashTimer -= ctx.deltaTime;
            if (pPlayerComponent->flashTimer <= 0.0f)
            {
                pPlayerComponent->flashTimer = 0.3f;
                polylineComponents[pPlayerComponent->playerPolylineComponent]->visible = !polylineComponents[pPlayerComponent->playerPolylineComponent]->visible;
            }
        }
    }
}
