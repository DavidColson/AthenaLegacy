#include "Systems.h"

#include "Components/Components.h"

#include <Renderer/Renderer.h>
#include <Input/Input.h>

void CollisionSystemUpdate(Space* space, float deltaTime)
{
	// Need to compare two entities at a time?
	// Can't do this
}

void AsteroidSystemUpdate(Space* space, float deltaTime)
{

}

void DrawShapeSystem(Space* space, float deltaTime)
{
	for (EntityID id : View<CTransform, CDrawable>(space))
	{
		CTransform* pTransform = space->GetComponent<CTransform>(id);
		CDrawable* pDrawable = space->GetComponent<CDrawable>(id);

		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
		pDrawable->m_renderProxy.m_lineThickness = pDrawable->m_lineThickness;

		Graphics::SubmitProxy(&pDrawable->m_renderProxy);
	}
}

void MovementSystemUpdate(Space* space, float deltaTime)
{
	for (EntityID id : View<CTransform>(space))
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
}

void ShipControlSystemUpdate(Space* space, float deltaTime)
{
	for (EntityID id : View<CTransform, CPlayerControl>(space))
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

		// Shoot a bullet
		if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
		{
			EntityID bullet = space->NewEntity();
			CBullet* pBullet = space->AssignComponent<CBullet>(bullet);

			CTransform* pBulletTrans = space->AssignComponent<CTransform>(bullet);
			pBulletTrans->m_pos = pTransform->m_pos;
			vec3 travelDir = vec3(-cos(pTransform->m_rot), -sin(pTransform->m_rot), 0.0f);
			pBulletTrans->m_vel = pTransform->m_vel + travelDir * pBullet->m_speed;
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
}
