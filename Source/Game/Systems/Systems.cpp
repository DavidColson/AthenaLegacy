#include "Systems.h"

#include "Asteroids.h"
#include "Components/Components.h"

#include <Renderer/DebugDraw.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderFont.h>
#include <Input/Input.h>
#include <Utility.h>


// **********
// Events/Functions
// **********

void SpawnBullet(Scene* pScene, const CTransform* pAtTransform)
{
	EntityID bullet = pScene->NewEntity("Bullet");
	CBullet* pBullet = pScene->AssignComponent<CBullet>(bullet);

	CTransform* pBulletTrans = pScene->AssignComponent<CTransform>(bullet);
	pBulletTrans->m_pos = pAtTransform->m_pos;
	Vec3f travelDir = Vec3f(-cos(pAtTransform->m_rot), -sin(pAtTransform->m_rot), 0.0f);
	pBulletTrans->m_vel = pAtTransform->m_vel + travelDir * pBullet->m_speed;
	pBulletTrans->m_rot = pAtTransform->m_rot;

	CDrawable* pDrawable = pScene->AssignComponent<CDrawable>(bullet);
	pDrawable->m_renderProxy = RenderProxy(
		{
			Vertex(Vec3f(0.f, 0.0f, 0.f)),
			Vertex(Vec3f(0.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 0.0f, 0.f)),
		}, {
			// Note, has adjacency data
			3, 0, 1, 2, 3, 0, 1
		});
	pDrawable->m_lineThickness = 3.0f;
	pScene->AssignComponent<CCollidable>(bullet)->m_radius = 4.0f;
}


void OnBulletAsteroidCollision(Scene* pScene, EntityID bullet, EntityID asteroid)
{
	Log::Print(Log::EMsg, "Bullet touched asteroid between \"%i - %s\" and \"%i - %s\"", GetEntityIndex(bullet), pScene->GetEntityName(bullet), GetEntityIndex(asteroid), pScene->GetEntityName(asteroid));

	// Do scoring for the player
	// #RefactorNote: Another use of single entity for loop lookup
	// Consider player storing the entity Id of the score entity. Or a HUD singleton storing the entity ID.
	for (EntityID score : SceneView<CPlayerScore, CText>(pScene))
	{
		int hits = pScene->GetComponent<CAsteroid>(asteroid)->m_hitCount;

		CPlayerScore* pPlayerScore = pScene->GetComponent<CPlayerScore>(score);
		CText* pText = pScene->GetComponent<CText>(score);

		// #RefactorNote: Get rid of branching here by storing the score for an asteroid in the asteroid
		if (hits == 0)
			pPlayerScore->m_score += 20;
		
		if (hits == 1)
			pPlayerScore->m_score += 50;

		if (hits == 2)
			pPlayerScore->m_score += 100;
			
		pScene->GetComponent<CText>(score)->m_text = StringFormat("%i", pPlayerScore->m_score);
	}

	// Asteroid vs bullet collision
	if (pScene->GetComponent<CAsteroid>(asteroid)->m_hitCount >= 2) // Smallest type asteroid, destroy and return
	{
		pScene->DestroyEntity(asteroid);
		return;
	}

	CTransform* pTransform = pScene->GetComponent<CTransform>(asteroid);
	CCollidable* pCollidable = pScene->GetComponent<CCollidable>(asteroid);
	for (int i = 0; i < 2; i++)
	{
		auto randf = []() { return float(rand()) / float(RAND_MAX); };
		float randomRotation = randf() * 6.282f;

		Vec3f randomVelocity = pTransform->m_vel + Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f) * 80.0f;

		EntityID newAsteroid = pScene->NewEntity("Asteroid");
		pScene->AssignComponent<CCollidable>(newAsteroid)->m_radius = pCollidable->m_radius * 0.5f;
		CTransform* pNewTransform = pScene->AssignComponent<CTransform>(newAsteroid);
		pNewTransform->m_pos = pTransform->m_pos;
		pNewTransform->m_sca = pTransform->m_sca * 0.5f;
		pNewTransform->m_vel = randomVelocity;
		pNewTransform->m_rot = randomRotation;

		pScene->AssignComponent<CDrawable>(newAsteroid)->m_renderProxy = Game::g_asteroidMeshes[rand() % 4];
		pScene->AssignComponent<CAsteroid>(newAsteroid)->m_hitCount = pScene->GetComponent<CAsteroid>(asteroid)->m_hitCount + 1;
	}

	pScene->DestroyEntity(asteroid);
	pScene->DestroyEntity(bullet);
}

void OnPlayerAsteroidCollision(Scene* pScene, EntityID player, EntityID asteroid)
{
	if (!pScene->HasComponent<CDrawable>(player))
		return; // Can't kill a player who is dead


	CPlayerControl* pPlayerControl = pScene->GetComponent<CPlayerControl>(player);
	pScene->RemoveComponent<CDrawable>(player);
	pPlayerControl->m_lives -= 1;
	pScene->DestroyEntity(pPlayerControl->m_lifeEntities[pPlayerControl->m_lives]);
	
	Log::Print(Log::EMsg, "Player died");
	
	if (pPlayerControl->m_lives <= 0)
	{
		// #RefactorNote: We're iterating the entire list of entities to find the one we want to tell it to be invisible.
		// Maybe this isn't ideal. Maybe we should have a system for updating UI elements that can figure out when stuff
		// Needs updating and do it there.
		// Could have a HUD singleton component, that stores entity Ids for each hud component to update
		// Or player's themselves store a refernece to the game over entity
		for (EntityID gameOver : SceneView<CText, CGameOver>(pScene))
		{
			pScene->GetComponent<CText>(gameOver)->m_visible = true;
		}
		return; // Game over
	} 

	pPlayerControl->m_respawnTimer = 5.0f;
	pScene->DestroyEntity(asteroid);

	float w = Graphics::GetContext()->m_windowWidth;
	float h = Graphics::GetContext()->m_windowHeight;
	CTransform* pTransform = pScene->GetComponent<CTransform>(player);
	pTransform->m_pos = Vec3f(w/2.0f, h/2.0f, 0.0f);
	pTransform->m_rot = 0.0f;
	pTransform->m_vel = Vec3f(0.0f, 0.0f, 0.0f);
	pTransform->m_accel = Vec3f(0.0f, 0.0f, 0.0f);
}

// **********
// SYSTEMS
// **********

void CollisionSystemUpdate(Scene* pScene, float deltaTime)
{
	bool continueOuter = false;
	for (EntityID entity1 : SceneView<CTransform, CCollidable>(pScene))
	{
		Vec2f pos = Vec2f::Project3D(pScene->GetComponent<CTransform>(entity1)->m_pos);
		float radius = pScene->GetComponent<CCollidable>(entity1)->m_radius;
		//DebugDraw::Draw2DCircle(pos, radius, Vec3f(1.0f, 0.0f, 0.0f));

		for (EntityID entity2 : SceneView<CTransform, CCollidable>(pScene))
		{
			// Don't collide with yourself
			if (entity1 == entity2)
				continue;
			// Asteroids don't collide with each other
			if (pScene->HasComponent<CAsteroid>(entity1) && pScene->HasComponent<CAsteroid>(entity2))
				continue;

			CCollidable* pCollider1 = pScene->GetComponent<CCollidable>(entity1);
			CCollidable* pCollider2 = pScene->GetComponent<CCollidable>(entity2);

			float distance = (pScene->GetComponent<CTransform>(entity1)->m_pos - pScene->GetComponent<CTransform>(entity2)->m_pos).GetLength();
			float collisionDistance = pCollider1->m_radius + pCollider2->m_radius;
			
			if (distance < collisionDistance)
			{
				if (pScene->HasComponent<CBullet>(entity1) && pScene->HasComponent<CAsteroid>(entity2))
				{
					OnBulletAsteroidCollision(pScene, entity1, entity2);
					continueOuter = true; break; // the bullet will have been deleted, so skip this iteration
				}

				if (pScene->HasComponent<CPlayerControl>(entity1) && pScene->HasComponent<CAsteroid>(entity2))
				{
					OnPlayerAsteroidCollision(pScene, entity1, entity2);
				}
			}
		}
		if (continueOuter)
			continue;
	}
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

void DrawTextSystem(Scene* pScene, float deltaTime)
{
	for (EntityID id : SceneView<CTransform, CText>(pScene))
	{
		CText* pText = pScene->GetComponent<CText>(id);
		if (pText->m_visible) // #RefactorNote: Consider making visible a component, 
			// and then you can just iterate over visible renderables, less branching
		{
			CTransform* pTransform = pScene->GetComponent<CTransform>(id);
			Graphics::GetContext()->m_pFontRender->SubmitText(pText->m_text.c_str(), Vec2f(pTransform->m_pos.x, pTransform->m_pos.y));
		}
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

		// #RefactorNote: Probably clearer to have an "isDead" variable on PlayerControl, and check that here instead
		if (!pScene->HasComponent<CDrawable>(id))
		{
			if (pControl->m_respawnTimer > 0.0f)
			{
				pControl->m_respawnTimer -= deltaTime;
				if (pControl->m_respawnTimer <= 0.0f)
				{
					// #RefactorNote: Assign and remove a visibility component instead
					pScene->AssignComponent<CDrawable>(id)->m_renderProxy = Game::g_shipMesh;
				}
			}
			return; // Player not being drawn is dead
		}

		Vec3f accel(0.0f, 0.0f, 0.0f);

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
			SpawnBullet(pScene, pTransform);
		}
	}
}
