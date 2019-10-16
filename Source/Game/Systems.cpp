#include "Systems.h"

#include "Asteroids.h"
#include "Components.h"

#include <Renderer/DebugDraw.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderFont.h>
#include <Input/Input.h>
#include <Utility.h>
#include <Profiler.h>


// **********
// Events/Functions
// **********

void SpawnBullet(Scene& scene, const CTransform* pAtTransform)
{
	EntityID bullet = scene.NewEntity("Bullet");
	CBullet* pBullet = scene.Assign<CBullet>(bullet);

	CTransform* pBulletTrans = scene.Assign<CTransform>(bullet);
	pBulletTrans->pos = pAtTransform->pos;
	Vec3f travelDir = Vec3f(-cos(pAtTransform->rot), -sin(pAtTransform->rot), 0.0f);
	pBulletTrans->vel = pAtTransform->vel + travelDir * pBullet->speed;
	pBulletTrans->rot = pAtTransform->rot;

	CDrawable* pDrawable = scene.Assign<CDrawable>(bullet);
	pDrawable->renderProxy = RenderProxy(
		{
			Vertex(Vec3f(0.f, 0.0f, 0.f)),
			Vertex(Vec3f(0.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 0.0f, 0.f)),
		}, {
			// Note, has adjacency data
			3, 0, 1, 2, 3, 0, 1
		});
	pDrawable->lineThickness = 3.0f;
	scene.Assign<CCollidable>(bullet)->radius = 4.0f;
}


void OnBulletAsteroidCollision(Scene& scene, EntityID bullet, EntityID asteroid)
{
	Log::Print(Log::EMsg, "Bullet touched asteroid between \"%i - %s\" and \"%i - %s\"", GetEntityIndex(bullet), scene.GetEntityName(bullet), GetEntityIndex(asteroid), scene.GetEntityName(asteroid));

	// Do scoring for the player
	// #RefactorNote: Another use of single entity for loop lookup
	// Consider player storing the entity Id of the score entity. Or a HUD singleton storing the entity ID.
	for (EntityID score : SceneView<CPlayerScore, CText>(scene))
	{
		int hits = scene.Get<CAsteroid>(asteroid)->hitCount;

		CPlayerScore* pPlayerScore = scene.Get<CPlayerScore>(score);
		CText* pText = scene.Get<CText>(score);

		// #RefactorNote: Get rid of branching here by storing the score for an asteroid in the asteroid
		if (hits == 0)
			pPlayerScore->score += 20;
		
		if (hits == 1)
			pPlayerScore->score += 50;

		if (hits == 2)
			pPlayerScore->score += 100;
			
		scene.Get<CText>(score)->text = StringFormat("%i", pPlayerScore->score);
	}

	// Asteroid vs bullet collision
	if (scene.Get<CAsteroid>(asteroid)->hitCount >= 2) // Smallest type asteroid, destroy and return
	{
		scene.DestroyEntity(asteroid);
		return;
	}

	CTransform* pTransform = scene.Get<CTransform>(asteroid);
	CCollidable* pCollidable = scene.Get<CCollidable>(asteroid);
	for (int i = 0; i < 2; i++)
	{
		auto randf = []() { return float(rand()) / float(RAND_MAX); };
		float randomRotation = randf() * 6.282f;

		Vec3f randomVelocity = pTransform->vel + Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f) * 80.0f;

		EntityID newAsteroid = scene.NewEntity("Asteroid");
		scene.Assign<CCollidable>(newAsteroid)->radius = pCollidable->radius * 0.5f;
		CTransform* pNewTransform = scene.Assign<CTransform>(newAsteroid);
		pNewTransform->pos = pTransform->pos;
		pNewTransform->sca = pTransform->sca * 0.5f;
		pNewTransform->vel = randomVelocity;
		pNewTransform->rot = randomRotation;

		scene.Assign<CDrawable>(newAsteroid)->renderProxy = Game::g_asteroidMeshes[rand() % 4];
		scene.Assign<CAsteroid>(newAsteroid)->hitCount = scene.Get<CAsteroid>(asteroid)->hitCount + 1;
	}

	scene.DestroyEntity(asteroid);
	scene.DestroyEntity(bullet);
}

void OnPlayerAsteroidCollision(Scene& scene, EntityID player, EntityID asteroid)
{
	if (!scene.Has<CDrawable>(player))
		return; // Can't kill a player who is dead


	CPlayerControl* pPlayerControl = scene.Get<CPlayerControl>(player);
	scene.Remove<CDrawable>(player);
	pPlayerControl->lives -= 1;
	scene.DestroyEntity(pPlayerControl->lifeEntities[pPlayerControl->lives]);
	
	Log::Print(Log::EMsg, "Player died");
	
	if (pPlayerControl->lives <= 0)
	{
		// #RefactorNote: We're iterating the entire list of entities to find the one we want to tell it to be invisible.
		// Maybe this isn't ideal. Maybe we should have a system for updating UI elements that can figure out when stuff
		// Needs updating and do it there.
		// Could have a HUD singleton component, that stores entity Ids for each hud component to update
		// Or player's themselves store a refernece to the game over entity
		for (EntityID gameOver : SceneView<CText, CGameOver>(scene))
		{
			scene.Get<CText>(gameOver)->visible = true;
		}
		return; // Game over
	} 

	pPlayerControl->respawnTimer = 5.0f;
	scene.DestroyEntity(asteroid);

	float w = Graphics::GetContext()->windowWidth;
	float h = Graphics::GetContext()->windowHeight;
	CTransform* pTransform = scene.Get<CTransform>(player);
	pTransform->pos = Vec3f(w/2.0f, h/2.0f, 0.0f);
	pTransform->rot = 0.0f;
	pTransform->vel = Vec3f(0.0f, 0.0f, 0.0f);
	pTransform->accel = Vec3f(0.0f, 0.0f, 0.0f);
}

// **********
// SYSTEMS
// **********

void CollisionSystemUpdate(Scene& scene, float deltaTime)
{
	PROFILE();

	bool bContinueOuter = false;
	for (EntityID asteroid : SceneView<CAsteroid>(scene))
	{
		float asteroidRad = scene.Get<CCollidable>(asteroid)->radius;
		for (EntityID bullet : SceneView<CBullet>(scene))
		{
			float bulletRad = scene.Get<CCollidable>(bullet)->radius;

			float distance = (scene.Get<CTransform>(asteroid)->pos - scene.Get<CTransform>(bullet)->pos).GetLength();
			float collisionDistance = asteroidRad + bulletRad;
			
			if (distance < collisionDistance)
			{
				OnBulletAsteroidCollision(scene, bullet, asteroid);
				bContinueOuter = true; break;
			}
		}
		if(bContinueOuter) // The asteroid has been deleted, so skip
			continue;

		// #RefactorNote, get the player's ID from a singleton and completely avoid this loop
		for (EntityID player : SceneView<CPlayerControl>(scene))
		{
			float playerRad = scene.Get<CCollidable>(player)->radius;

			float distance = (scene.Get<CTransform>(asteroid)->pos - scene.Get<CTransform>(player)->pos).GetLength();
			float collisionDistance = asteroidRad + playerRad;
			
			if (distance < collisionDistance)
			{
				OnPlayerAsteroidCollision(scene, player, asteroid);
				bContinueOuter = true; break;
			}
		}
		if(bContinueOuter) // The asteroid has been deleted, so skip
			continue;
	}
}

void DrawShapeSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneView<CTransform, CDrawable>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CDrawable* pDrawable = scene.Get<CDrawable>(id);

		pDrawable->renderProxy.SetTransform(pTransform->pos, pTransform->rot, pTransform->sca);
		pDrawable->renderProxy.lineThickness = pDrawable->lineThickness;

		Graphics::SubmitProxy(&pDrawable->renderProxy);
	}
}

void DrawTextSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneView<CTransform, CText>(scene))
	{
		CText* pText = scene.Get<CText>(id);
		if (pText->visible) // #RefactorNote: Consider making visible a component, 
			// and then you can just iterate over visible renderables, less branching
		{
			CTransform* pTransform = scene.Get<CTransform>(id);
			Graphics::GetContext()->pFontRender->SubmitText(pText->text.c_str(), Vec2f(pTransform->pos.x, pTransform->pos.y));
		}
	}
}

void MovementSystemUpdate(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneView<CTransform>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);

		pTransform->vel = pTransform->vel + pTransform->accel * deltaTime;
		pTransform->pos = pTransform->pos + pTransform->vel * deltaTime;

		if (pTransform->pos.x < 0.0f)
		{
			pTransform->pos.x = Graphics::GetContext()->windowWidth;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->pos.x > Graphics::GetContext()->windowWidth)
		{
			pTransform->pos.x = 0.0f;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}

		if (pTransform->pos.y < 0.0f)
		{
			pTransform->pos.y = Graphics::GetContext()->windowHeight;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->pos.y > Graphics::GetContext()->windowHeight)
		{
			pTransform->pos.y = 0.0f;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
	}
}

void ShipControlSystemUpdate(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneView<CTransform, CPlayerControl>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CPlayerControl* pControl = scene.Get<CPlayerControl>(id);

		// #RefactorNote: Probably clearer to have an "isDead" variable on PlayerControl, and check that here instead
		if (!scene.Has<CDrawable>(id))
		{
			if (pControl->respawnTimer > 0.0f)
			{
				pControl->respawnTimer -= deltaTime;
				if (pControl->respawnTimer <= 0.0f)
				{
					// #RefactorNote: Assign and remove a visibility component instead
					scene.Assign<CDrawable>(id)->renderProxy = Game::g_shipMesh;
				}
			}
			return; // Player not being drawn is dead
		}

		Vec3f accel(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyHeld(SDL_SCANCODE_UP))
		{
			accel.x = cos(pTransform->rot);
			accel.y = sin(pTransform->rot);
			accel = accel * -pControl->thrust;
		}
		pTransform->accel = accel - pTransform->vel * pControl->dampening;

		if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
			pTransform->rot += pControl->rotateSpeed;
		if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
			pTransform->rot -= pControl->rotateSpeed;

		// Shoot a bullet
		if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
		{
			SpawnBullet(scene, pTransform);
		}
	}
}
