#include "Systems/ShipControl.h"
#include "Components/Components.h"

#include <Renderer/Renderer.h>
#include <Input/Input.h>

void SShipControl::UpdateEntity(EntityID id, Space* space, float deltaTime)
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
	pTransform->m_accel = accel - pTransform->m_vel * pControl->m_dampening;

	if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
		pTransform->m_rot += pControl->m_rotateSpeed;
	if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
		pTransform->m_rot -= pControl->m_rotateSpeed;

	if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
	{
		EntityID bullet = space->NewEntity();
		CTransform* pBulletTrans = space->AssignComponent<CTransform>(bullet);
		pBulletTrans->m_pos = pTransform->m_pos;
		vec3 travelDir = vec3(-cos(pTransform->m_rot), -sin(pTransform->m_rot), 0.0f);
		pBulletTrans->m_vel = pTransform->m_vel + travelDir * 600.0f;
		pBulletTrans->m_rot = pTransform->m_rot;

		CDrawable* pDrawable = space->AssignComponent<CDrawable>(bullet);
		pDrawable->m_renderProxy = RenderProxy(
			{
				Vertex(vec3(0.f, 0.0f, 0.f)),
				Vertex(vec3(0.f, 1.0f, 0.f)),
				Vertex(vec3(1.f, 1.0f, 0.f)),
				Vertex(vec3(1.f, 0.0f, 0.f)),
			}, {
				// Note, has adjacency data
				3, 0, 1, 2, 3, 0, 1
			});
		pDrawable->m_lineThickness = 3.0f;
		Graphics::SubmitProxy(&pDrawable->m_renderProxy);
	}
}

void SShipControl::SetSubscriptions()
{
	Subscribe<CTransform>();
	Subscribe<CPlayerControl>();
}

