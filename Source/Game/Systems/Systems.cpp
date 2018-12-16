#include "Systems.h"

#include "Components/Components.h"

#include <Renderer/DebugDraw.h>
#include <Renderer/Renderer.h>
#include <Input/Input.h>

void CollisionSystemUpdate(Scene* pScene, float deltaTime)
{
	for (EntityID entity : SceneView<CCollidable>(pScene))
	{
		CCollidable* pCollider = pScene->GetComponent<CCollidable>(entity);
		pCollider->m_colliding = false;
		pCollider->m_other = INVALID_ENTITY;
	}

	for (EntityID entity1 : SceneView<CTransform, CCollidable>(pScene))
	{
		vec2 pos = proj<2>(pScene->GetComponent<CTransform>(entity1)->m_pos);
		float radius = pScene->GetComponent<CCollidable>(entity1)->m_radius;
		DebugDraw::Draw2DCircle(pos, radius, vec3(1.0f, 0.0f, 0.0f));

		for (EntityID entity2 : SceneView<CTransform, CCollidable>(pScene))
		{
			CCollidable* pCollider1 = pScene->GetComponent<CCollidable>(entity1);
			CCollidable* pCollider2 = pScene->GetComponent<CCollidable>(entity2);

			float distance = (pScene->GetComponent<CTransform>(entity1)->m_pos - pScene->GetComponent<CTransform>(entity2)->m_pos).mag();
			float collisionDistance = pCollider1->m_radius + pCollider2->m_radius;
			
			if (distance < collisionDistance && entity1 != entity2)
			{
				pCollider1->m_colliding = true;
				pCollider1->m_other = entity2;
				pCollider2->m_colliding = true;
				pCollider2->m_other = entity1;
			}
		}
	}
}

void AsteroidSystemUpdate(Scene* pScene, float deltaTime)
{

}

void DrawShapeSystem(Scene* pScene, float deltaTime)
{
	for (EntityID id : SceneView<CTransform, CDrawable>(pScene))
	{
		CTransform* pTransform = pScene->GetComponent<CTransform>(id);
		CDrawable* pDrawable = pScene->GetComponent<CDrawable>(id);

		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
		pDrawable->m_renderProxy.m_lineThickness = pDrawable->m_lineThickness;

		Graphics::SubmitProxy(&pDrawable->m_renderProxy);
	}
}

void MovementSystemUpdate(Scene* pScene, float deltaTime)
{
	for (EntityID id : SceneView<CTransform>(pScene))
	{
		CTransform* pTransform = pScene->GetComponent<CTransform>(id);

		pTransform->m_vel = pTransform->m_vel + pTransform->m_accel * deltaTime;
		pTransform->m_pos = pTransform->m_pos + pTransform->m_vel * deltaTime;

		if (pTransform->m_pos.x < 0.0f)
		{
			pTransform->m_pos.x = Graphics::GetContext()->m_windowWidth;
			if (pScene->HasComponent<CBullet>(id)) pScene->DestroyEntity(id);
		}
		else if (pTransform->m_pos.x > Graphics::GetContext()->m_windowWidth)
		{
			pTransform->m_pos.x = 0.0f;
			if (pScene->HasComponent<CBullet>(id)) pScene->DestroyEntity(id);
		}

		if (pTransform->m_pos.y < 0.0f)
		{
			pTransform->m_pos.y = Graphics::GetContext()->m_windowHeight;
			if (pScene->HasComponent<CBullet>(id)) pScene->DestroyEntity(id);
		}
		else if (pTransform->m_pos.y > Graphics::GetContext()->m_windowHeight)
		{
			pTransform->m_pos.y = 0.0f;
			if (pScene->HasComponent<CBullet>(id)) pScene->DestroyEntity(id);
		}
	}
}

void ShipControlSystemUpdate(Scene* pScene, float deltaTime)
{
	for (EntityID id : SceneView<CTransform, CPlayerControl>(pScene))
	{
		CTransform* pTransform = pScene->GetComponent<CTransform>(id);
		CPlayerControl* pControl = pScene->GetComponent<CPlayerControl>(id);

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
			EntityID bullet = pScene->NewEntity();
			CBullet* pBullet = pScene->AssignComponent<CBullet>(bullet);

			CTransform* pBulletTrans = pScene->AssignComponent<CTransform>(bullet);
			pBulletTrans->m_pos = pTransform->m_pos;
			vec3 travelDir = vec3(-cos(pTransform->m_rot), -sin(pTransform->m_rot), 0.0f);
			pBulletTrans->m_vel = pTransform->m_vel + travelDir * pBullet->m_speed;
			pBulletTrans->m_rot = pTransform->m_rot;

			CDrawable* pDrawable = pScene->AssignComponent<CDrawable>(bullet);
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
