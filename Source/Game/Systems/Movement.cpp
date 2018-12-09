#include "Movement.h"

#include "Components/Components.h"

#include <Renderer/Renderer.h>

void SMovement::UpdateEntity(EntityID id, Space * space, float deltaTime)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);

	pTransform->m_vel = pTransform->m_vel + pTransform->m_accel * deltaTime;
	pTransform->m_pos = pTransform->m_pos + pTransform->m_vel * deltaTime;

	if (pTransform->m_pos.x < 0.0f)
	{
		pTransform->m_pos.x = Graphics::GetContext()->m_windowWidth;
		if (space->HasComponent<CBullet>(id)) space->DestroyEntity(id);
	}
	else if (pTransform->m_pos.x > Graphics::GetContext()->m_windowWidth)
	{
		pTransform->m_pos.x = 0.0f;
		if (space->HasComponent<CBullet>(id)) space->DestroyEntity(id);
	}

	if (pTransform->m_pos.y < 0.0f)
	{
		pTransform->m_pos.y = Graphics::GetContext()->m_windowHeight;
		if (space->HasComponent<CBullet>(id)) space->DestroyEntity(id);
	}
	else if (pTransform->m_pos.y > Graphics::GetContext()->m_windowHeight)
	{
		pTransform->m_pos.y = 0.0f;
		if (space->HasComponent<CBullet>(id)) space->DestroyEntity(id);
	}
}

void SMovement::SetSubscriptions()
{
	Subscribe<CTransform>();
}
