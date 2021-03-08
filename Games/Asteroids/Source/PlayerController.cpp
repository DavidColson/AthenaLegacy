#include "PlayerController.h"

#include <Input/Input.h>

#include <SDL_scancode.h>

REFLECT_BEGIN_DERIVED(PlayerComponent, IComponent)
REFLECT_MEMBER(thrust)
REFLECT_MEMBER(rotateSpeed)
REFLECT_MEMBER(dampening)
REFLECT_END()

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

void PlayerController::Update(float deltaTime)
{
    if (pRootPhysics && pPlayerComponent)
    {
        Vec3f accel(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyHeld(SDL_SCANCODE_UP))
		{
			accel.x = cosf(pRootPhysics->GetLocalRotation().z);
			accel.y = sinf(pRootPhysics->GetLocalRotation().z);
			accel = accel * -pPlayerComponent->thrust;
		}

		pRootPhysics->acceleration = accel - pRootPhysics->velocity * pPlayerComponent->dampening;

        Vec3f localRotation = pRootPhysics->GetLocalRotation();
		if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
			localRotation.z += pPlayerComponent->rotateSpeed * deltaTime;

		if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
			localRotation.z -= pPlayerComponent->rotateSpeed * deltaTime;

        pRootPhysics->SetLocalRotation(localRotation);
    }
}
