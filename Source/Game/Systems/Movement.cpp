#include "Movement.h"

#include "Components/Components.h"

#include <Input/Input.h>

void SMovement::UpdateEntity(EntityID id, Space * space)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);
	CPlayerControl* pControl = space->GetComponent<CPlayerControl>(id);
	if (Input::GetKeyHeld(SDL_SCANCODE_D))
		pTransform->m_pos.x += pControl->m_moveSpeed.x;
	if (Input::GetKeyHeld(SDL_SCANCODE_A))
		pTransform->m_pos.x -= pControl->m_moveSpeed.x;
	if (Input::GetKeyHeld(SDL_SCANCODE_W))
		pTransform->m_pos.y += pControl->m_moveSpeed.y;
	if (Input::GetKeyHeld(SDL_SCANCODE_S))
		pTransform->m_pos.y -= pControl->m_moveSpeed.y;
}

void SMovement::SetSubscriptions()
{
	Subscribe<CTransform>();
	Subscribe<CPlayerControl>();
}
