#include "Movement.h"

#include "Components/Components.h"

#include <Input/Input.h>

void SMovement::UpdateEntity(EntityID id, Space * space, float deltaTime)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);
	CPlayerControl* pControl = space->GetComponent<CPlayerControl>(id);
	
	if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
		pTransform->m_rot += pControl->m_rotateSpeed;
	if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
		pTransform->m_rot -= pControl->m_rotateSpeed;

	pTransform->m_pos = pTransform->m_pos + pTransform->m_vel * deltaTime;
}

void SMovement::SetSubscriptions()
{
	Subscribe<CTransform>();
	Subscribe<CPlayerControl>();
}
