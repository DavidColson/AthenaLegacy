#include "Systems.h"

#include "Asteroids.h"
#include "Components.h"

#include <AudioDevice.h>
#include <Renderer/DebugDraw.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderFont.h>
#include <Renderer/ParticlesSystem.h>
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

	scene.Assign<CVisibility>(bullet);
	CDrawable* pDrawable = scene.Assign<CDrawable>(bullet);
	pDrawable->vertices = {
			Vertex(Vec3f(0.f, 0.0f, 0.f)),
			Vertex(Vec3f(0.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 1.0f, 0.f)),
			Vertex(Vec3f(1.f, 0.0f, 0.f)),
		};
	pDrawable->indices = { 3, 0, 1, 2, 3, 0, 1 };
	pDrawable->lineThickness = 5.0f;
	scene.Assign<CCollidable>(bullet)->radius = 4.0f;
}


void OnBulletAsteroidCollision(Scene& scene, EntityID bullet, EntityID asteroid)
{
	Log::Print(Log::EMsg, "Bullet touched asteroid between \"%i - %s\" and \"%i - %s\"", GetEntityIndex(bullet), scene.GetEntityName(bullet).c_str(), GetEntityIndex(asteroid), scene.GetEntityName(asteroid).c_str());

	// Do scoring for the player
	EntityID scoreEnt = scene.Get<CPlayerUI>(PLAYER_ID)->currentScoreEntity;
	CPlayerScore* pPlayerScore = scene.Get<CPlayerScore>(scoreEnt);
	CText* pText = scene.Get<CText>(scoreEnt);

	int hits = scene.Get<CAsteroid>(asteroid)->hitCount;
	if (hits == 0)
		pPlayerScore->score += 20;
	if (hits == 1)
		pPlayerScore->score += 50;
	if (hits == 2)
		pPlayerScore->score += 100;

	AudioDevice::PlaySound(scene.Get<CSounds>(PLAYER_ID)->explosionSound, 1.0f, false);
		
	scene.Get<CText>(scoreEnt)->text = StringFormat("%i", pPlayerScore->score);

	// Spawn death particles
	EntityID particles = scene.NewEntity("Asteroid Particles");
	scene.Assign<CTransform>(particles)->pos = scene.Get<CTransform>(asteroid)->pos;
	scene.Assign<CParticleEmitter>(particles);

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

		CDrawable* pDrawable = scene.Assign<CDrawable>(newAsteroid);
		int mesh = rand() % 4;
		pDrawable->vertices = Game::g_asteroidMeshes[mesh].vertices;
		pDrawable->indices = Game::g_asteroidMeshes[mesh].indices;
		scene.Assign<CVisibility>(newAsteroid);
		scene.Assign<CAsteroid>(newAsteroid)->hitCount = scene.Get<CAsteroid>(asteroid)->hitCount + 1;
	}

	scene.DestroyEntity(asteroid);
	scene.DestroyEntity(bullet);
}

void OnPlayerAsteroidCollision(Scene& scene, EntityID player, EntityID asteroid)
{
	if (scene.Get<CVisibility>(player)->visible == false)
		return; // Can't kill a player who is dead

	CPlayerControl* pPlayerControl = scene.Get<CPlayerControl>(player);
	scene.Get<CVisibility>(player)->visible = false;
	pPlayerControl->lives -= 1;
	scene.DestroyEntity(pPlayerControl->lifeEntities[pPlayerControl->lives]);
	AudioDevice::PauseSound(pPlayerControl->enginePlayingSound);

	Log::Print(Log::EMsg, "Player died");
	
	if (pPlayerControl->lives <= 0)
	{
		EntityID gameOverEnt = scene.Get<CPlayerUI>(player)->gameOverEntity;
		scene.Get<CVisibility>(gameOverEnt)->visible = true;
		return; // Game over
	} 

	pPlayerControl->respawnTimer = 5.0f;
	scene.DestroyEntity(asteroid);

	float w = GfxDevice::GetWindowWidth();
	float h = GfxDevice::GetWindowHeight();
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
		CTransform* pTrans = scene.Get<CTransform>(asteroid);

		//DebugDraw::Draw2DCircle(scene, Vec2f(pTrans->pos.x, pTrans->pos.y), asteroidRad + 10.0f, Vec3f(1.0f, 0.0f, 0.0f));

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

		if (!scene.Has<CInvincibility>(PLAYER_ID))
		{
			float playerRad = scene.Get<CCollidable>(PLAYER_ID)->radius;
			float distance = (scene.Get<CTransform>(asteroid)->pos - scene.Get<CTransform>(PLAYER_ID)->pos).GetLength();
			float collisionDistance = asteroidRad + playerRad;
			
			if (distance < collisionDistance)
			{
				OnPlayerAsteroidCollision(scene, PLAYER_ID, asteroid);
				continue;
			}
		}
	}
}

void InvincibilitySystemUpdate(Scene& scene, float deltaTime)
{
	PROFILE();

	if (scene.Has<CInvincibility>(PLAYER_ID))
	{
		CInvincibility* pInvincibility = scene.Get<CInvincibility>(PLAYER_ID);
		pInvincibility->m_timer -= deltaTime;
		if (pInvincibility->m_timer <= 0.0f)
		{
			scene.Remove<CInvincibility>(PLAYER_ID);
			return;
		}

		pInvincibility->m_flashTimer -= deltaTime;
		if (pInvincibility->m_flashTimer <= 0.0f)
		{
			pInvincibility->m_flashTimer = 0.3f;
			CVisibility* visiblity = scene.Get<CVisibility>(PLAYER_ID);
			if (visiblity->visible)
				visiblity->visible = false;
			else
				visiblity->visible = true;
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
			pTransform->pos.x = GfxDevice::GetWindowWidth();
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->pos.x > GfxDevice::GetWindowWidth())
		{
			pTransform->pos.x = 0.0f;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}

		if (pTransform->pos.y < 0.0f)
		{
			pTransform->pos.y = GfxDevice::GetWindowHeight();
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->pos.y > GfxDevice::GetWindowHeight())
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

		// If player doesn't have a drawable component we consider them dead
		if (scene.Get<CVisibility>(id)->visible == false && !scene.Has<CInvincibility>(id))
		{
			if (pControl->respawnTimer > 0.0f)
			{
				pControl->respawnTimer -= deltaTime;
				if (pControl->respawnTimer <= 0.0f)
				{
					scene.Get<CVisibility>(id)->visible = true;
					scene.Assign<CInvincibility>(id);
				}
			}
			return; // Player not being drawn and without invisibility is dead
		}

		Vec3f accel(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyHeld(SDL_SCANCODE_UP))
		{
			accel.x = cos(pTransform->rot);
			accel.y = sin(pTransform->rot);
			accel = accel * -pControl->thrust;
		}

		if (Input::GetKeyDown(SDL_SCANCODE_UP))
			AudioDevice::UnPauseSound(pControl->enginePlayingSound);
		else if (Input::GetKeyUp(SDL_SCANCODE_UP))
			AudioDevice::PauseSound(pControl->enginePlayingSound);

		pTransform->accel = accel - pTransform->vel * pControl->dampening;

		if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
			pTransform->rot += pControl->rotateSpeed;
		if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
			pTransform->rot -= pControl->rotateSpeed;

		// Shoot a bullet
		if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
		{
			SpawnBullet(scene, pTransform);
			AudioDevice::PlaySound(scene.Get<CSounds>(PLAYER_ID)->shootSound, 1.0f, false);
		}
	}
}
