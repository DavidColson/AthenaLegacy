#include "Movement.h"

#include "Components/Components.h"

#include <Input/Input.h>
#include <Renderer/Renderer.h>

void SMovement::UpdateEntity(EntityID id, Space * space, float deltaTime)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);
	CPlayerControl* pControl = space->GetComponent<CPlayerControl>(id);
	
	vec3 accel(0.0f, 0.0f, 0.0f);

	if (Input::GetKeyHeld(SDL_SCANCODE_UP))
	{
		accel.x = cos(pTransform->m_rot);
		accel.y = sin(pTransform->m_rot);
		accel = accel * -pControl->m_thrust;
	}
	accel = accel - pTransform->m_vel * pControl->m_dampening;

	if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
		pTransform->m_rot += pControl->m_rotateSpeed;
	if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
		pTransform->m_rot -= pControl->m_rotateSpeed;


	pTransform->m_vel = pTransform->m_vel + accel * deltaTime;
	pTransform->m_pos = pTransform->m_pos + pTransform->m_vel * deltaTime;

	if (pTransform->m_pos.x < 0.0f)
		pTransform->m_pos.x = Graphics::GetContext()->m_windowWidth;
	else if (pTransform->m_pos.x > Graphics::GetContext()->m_windowWidth)
		pTransform->m_pos.x = 0.0f;

	if (pTransform->m_pos.y < 0.0f)
		pTransform->m_pos.y = Graphics::GetContext()->m_windowHeight;
	else if (pTransform->m_pos.y > Graphics::GetContext()->m_windowHeight)
		pTransform->m_pos.y = 0.0f;
}

void SMovement::SetSubscriptions()
{
	Subscribe<CTransform>();
	Subscribe<CPlayerControl>();
}
