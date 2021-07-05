#include "PlayerController.h"

#include <World.h>
#include <Engine.h>
#include <Input/Input.h>

#include <SDL_scancode.h>

#include "../Asteroids.h"
#include "../Components.h"

void PlayerController::Activate()
{

}

void PlayerController::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<AsteroidPhysics>())
	{
		pRootPhysics = static_cast<AsteroidPhysics*>(pComponent);
	}

    if (pComponent->GetTypeData() == TypeDatabase::Get<PlayerComponent>())
    {
        pPlayerComponent = static_cast<PlayerComponent*>(pComponent);
    }
}

void PlayerController::UnregisterComponent(IComponent* pComponent)
{
    if (pComponent == pRootPhysics)
    {
        pRootPhysics = nullptr;
    }

    if (pComponent == pPlayerComponent)
    {
        pPlayerComponent = nullptr;
    }
}

void PlayerController::SpawnBullet(World* pWorld)
{
    Entity* pBullet = pWorld->NewEntity("Bullet");
    AsteroidPhysics* pPhysics = pBullet->AddNewComponent<AsteroidPhysics>();
    pPhysics->SetLocalPosition(pRootPhysics->GetLocalPosition());
    pPhysics->SetLocalRotation(pRootPhysics->GetLocalRotation());
    pPhysics->SetLocalScale(Vec3f(7.0f));
    pPhysics->type = CollisionType::Bullet;

	Vec3f travelDir = Vec3f(-cosf(pRootPhysics->GetLocalRotation().z), -sinf(pRootPhysics->GetLocalRotation().z), 0.0f);

    pPhysics->velocity = pRootPhysics->velocity + travelDir * 700.0f;
    pPhysics->collisionRadius = 4.0f;
    pPhysics->wrapAtEdge = false;

    Polyline* pPolyline = pBullet->AddNewComponent<Polyline>(pPhysics->GetId());
    pPolyline->points = {
		Vec2f(0.f, 0.0f),
		Vec2f(0.f, 1.0f)
	};
    pPolyline->thickness = 17.0f;
    pPolyline->connected = false;
}

void PlayerController::Update(UpdateContext& ctx)
{
    if (Input::GetKeyDown(SDL_SCANCODE_ESCAPE))
    {
        LoadMenu();
    }

    if (pRootPhysics && pPlayerComponent)
    {
        if (pPlayerComponent->lives.empty() && Input::GetKeyDown(SDL_SCANCODE_RETURN))
        {
            LoadMainScene();
        }

        if (pPlayerComponent->respawnTimer > 0.0f)
        {
            return;
        }

        Vec3f accel(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyHeld(SDL_SCANCODE_UP))
		{
			accel.x = cosf(pRootPhysics->GetLocalRotation().z);
			accel.y = sinf(pRootPhysics->GetLocalRotation().z);
			accel = accel * -pPlayerComponent->thrust;
		}

        if (Input::GetKeyDown(SDL_SCANCODE_UP))
			AudioDevice::UnPauseSound(pPlayerComponent->enginePlayingSound);
		else if (Input::GetKeyUp(SDL_SCANCODE_UP))
			AudioDevice::PauseSound(pPlayerComponent->enginePlayingSound);

		pRootPhysics->acceleration = accel - pRootPhysics->velocity * pPlayerComponent->dampening;

        Vec3f localRotation = pRootPhysics->GetLocalRotation();
		if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
			localRotation.z += pPlayerComponent->rotateSpeed * ctx.deltaTime;

		if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
			localRotation.z -= pPlayerComponent->rotateSpeed * ctx.deltaTime;

        pRootPhysics->SetLocalRotation(localRotation);

        if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
		{
			SpawnBullet(ctx.pWorld);
			AudioDevice::PlaySound(pPlayerComponent->shootSound, 1.0f, false);
		}
    }
}
