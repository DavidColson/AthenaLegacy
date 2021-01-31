#include "Systems.h"

#include "Asteroids.h"
#include "Components.h"

#include <AudioDevice.h>
#include <Rendering/FontSystem.h>
#include <Rendering/ParticlesSystem.h>
#include <Rendering/GameRenderer.h>
#include <Input/Input.h>
#include <Profiler.h>
#include <Vec4.h>

#include <SDL_scancode.h>


// **********
// Events/Functions
// **********

void SpawnBullet(Scene& scene, const CTransform* pAtTransform, const Vec3f atVelocity)
{
	EntityID bullet = scene.NewEntity("Bullet");
	CBullet* pBullet = scene.Assign<CBullet>(bullet);

	CTransform* pBulletTrans = scene.Assign<CTransform>(bullet);
	pBulletTrans->localPos = pAtTransform->localPos;
	Vec3f travelDir = Vec3f(-cosf(pAtTransform->localRot.z), -sinf(pAtTransform->localRot.z), 0.0f);
	pBulletTrans->localRot = pAtTransform->localRot;

	CDynamics* pDynamics = scene.Assign<CDynamics>(bullet);
	pDynamics->vel = atVelocity + travelDir * pBullet->speed;

	scene.Assign<CVisibility>(bullet);
	scene.Assign<CCollidable>(bullet)->radius = 4.0f;
}


void OnBulletAsteroidCollision(Scene& scene, EntityID bullet, EntityID asteroid)
{
	Log::Info("Bullet touched asteroid between \"%i - %s\" and \"%i - %s\"", bullet.Index(), scene.GetEntityName(bullet).c_str(), asteroid.Index(), scene.GetEntityName(asteroid).c_str());

	// Do scoring for the player
	EntityID scoreEnt = scene.Get<CPlayerUI>(PLAYER_ID)->currentScoreEntity;
	CPlayerScore* pPlayerScore = scene.Get<CPlayerScore>(scoreEnt);

	int hits = scene.Get<CAsteroid>(asteroid)->hitCount;
	if (hits == 0)
		pPlayerScore->score += 20;
	if (hits == 1)
		pPlayerScore->score += 50;
	if (hits == 2)
		pPlayerScore->score += 100;

	AudioDevice::PlaySound(scene.Get<CSounds>(PLAYER_ID)->explosionSound, 1.0f, false);
		
	scene.Get<CText>(scoreEnt)->text.sprintf("%i", pPlayerScore->score);

	// Spawn death particles
	EntityID particles = scene.NewEntity("Asteroid Particles");
	scene.Assign<CTransform>(particles)->localPos = scene.Get<CTransform>(asteroid)->localPos;
	scene.Assign<CParticleEmitter>(particles);

	// Asteroid vs bullet collision
	if (scene.Get<CAsteroid>(asteroid)->hitCount >= 2) // Smallest type asteroid, destroy and return
	{
		scene.DestroyEntity(asteroid);
		return;
	}

	CTransform* pTransform = scene.Get<CTransform>(asteroid);
	CDynamics* pDynamics = scene.Get<CDynamics>(asteroid);
	CCollidable* pCollidable = scene.Get<CCollidable>(asteroid);
	for (int i = 0; i < 2; i++)
	{
		auto randf = []() { return float(rand()) / float(RAND_MAX); };
		float randomRotation = randf() * 6.282f;

		Vec3f randomVelocity = pDynamics->vel + Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f) * 80.0f;

		EntityID newAsteroid = scene.NewEntity("Asteroid");
		scene.Assign<CCollidable>(newAsteroid)->radius = pCollidable->radius * 0.5f;
		CTransform* pNewTransform = scene.Assign<CTransform>(newAsteroid);
		pNewTransform->localPos = pTransform->localPos;
		pNewTransform->localSca = pTransform->localSca * 0.5f;
		pNewTransform->localRot = randomRotation;

		scene.Assign<CDynamics>(newAsteroid)->vel = randomVelocity;

		scene.Assign<CVisibility>(newAsteroid);
		CAsteroid* pAsteroid = scene.Assign<CAsteroid>(newAsteroid);
		pAsteroid->hitCount = scene.Get<CAsteroid>(asteroid)->hitCount + 1;
		pAsteroid->type = AsteroidType(rand() % 4);
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

	Log::Info("Player died");
	
	if (pPlayerControl->lives <= 0)
	{
		EntityID gameOverEnt = scene.Get<CPlayerUI>(player)->gameOverEntity;
		scene.Get<CVisibility>(gameOverEnt)->visible = true;
		return; // Game over
	} 

	pPlayerControl->respawnTimer = 5.0f;
	scene.DestroyEntity(asteroid);

	float w = GameRenderer::GetWidth();
	float h = GameRenderer::GetHeight();
	CTransform* pTransform = scene.Get<CTransform>(player);
	pTransform->localPos = Vec3f(w/2.0f, h/2.0f, 0.0f);
	pTransform->localRot = 0.0f;
	CDynamics* pDynamics = scene.Get<CDynamics>(player);
	pDynamics->vel = Vec3f(0.0f, 0.0f, 0.0f);
	pDynamics->accel = Vec3f(0.0f, 0.0f, 0.0f);
}

// **********
// SYSTEMS
// **********

void DrawAsteroids(Scene& scene, float /* deltaTime */)
{
	PROFILE();

	for (EntityID shape : SceneIterator<CAsteroid, CTransform, CVisibility>(scene))
	{
		if (scene.Get<CVisibility>(shape)->visible == false)
			continue;

		CTransform* pTrans = scene.Get<CTransform>(shape);
		Matrixf pivotAdjust = Matrixf::MakeTranslation(Vec3f(-0.5f, -0.5f, 0.0f));
		Matrixf world = Matrixf::MakeTRS(pTrans->localPos, Vec3f(0.0f, 0.0f, pTrans->localRot.z), pTrans->localSca) * pivotAdjust;

		CAsteroid* pAsteroid = scene.Get<CAsteroid>(shape);
		GfxDraw::SetTransform(world);
		GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::ZAlign);
		GfxDraw::Polyshape(GetAsteroidMesh(pAsteroid->type));
		GfxDraw::SetTransform(Matrixf::Identity());
	}	
}

void DrawBullets(Scene& scene, float deltaTime)
{
	for (EntityID shape : SceneIterator<CBullet, CTransform, CVisibility>(scene))
	{
		if (scene.Get<CVisibility>(shape)->visible == false)
			continue;

		CTransform* pTrans = scene.Get<CTransform>(shape);
		Matrixf world = Matrixf::MakeTRS(pTrans->localPos, Vec3f(0.0f, 0.0f, pTrans->localRot.z), pTrans->localSca);

		GfxDraw::Paint bulletPaint;
		bulletPaint.drawStyle = GfxDraw::DrawStyle::Stroke;
		bulletPaint.strokeThickness = 10.f;
		bulletPaint.strokeColor = Vec4f(1.0f);

		GfxDraw::SetTransform(world);
		GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::ZAlign);
		GfxDraw::Rect(Vec3f(0.0f), Vec2f(10.0f, 10.0f), Vec4f(0.0f), bulletPaint);
		GfxDraw::SetTransform(Matrixf::Identity());
	}	
}

void DrawShips(Scene& scene, float deltaTime)
{
	for (EntityID shape : SceneIterator<CShipDraw, CTransform, CVisibility>(scene))
	{
		if (scene.Get<CVisibility>(shape)->visible == false)
			continue;

		CTransform* pTrans = scene.Get<CTransform>(shape);
		Matrixf posMat = Matrixf::MakeTranslation(pTrans->localPos);
		Matrixf rotMat = Matrixf::MakeRotation(Vec3f(0.0f, 0.0f, pTrans->localRot.z));
		Matrixf scaMat = Matrixf::MakeScale(pTrans->localSca);
		Matrixf pivotAdjust = Matrixf::MakeTranslation(Vec3f(-0.5f, -0.5f, 0.0f));
		Matrixf world = posMat * rotMat * scaMat * pivotAdjust;

		GfxDraw::SetTransform(world);
		GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::ZAlign);
		GfxDraw::Polyshape(GetPlayerMesh());
		GfxDraw::SetTransform(Matrixf::Identity());
	}	
}

void AsteroidSpawning(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID spawnerEnt : SceneIterator<CAsteroidSpawner>(scene))
	{
		CAsteroidSpawner& spawner = *(scene.Get<CAsteroidSpawner>(spawnerEnt));

		if (scene.Get<CVisibility>(PLAYER_ID)->visible == false)
			continue;

		spawner.timer -= deltaTime;

		if (spawner.timer <= 0.0f)
		{
			spawner.timer = spawner.timeBetweenSpawns;

			if (spawner.timeBetweenSpawns > 1.0f)
				spawner.timeBetweenSpawns *= spawner.decay;
			else
				spawner.timeBetweenSpawns = 1.0f;
			

			// Actually create an asteroid
			if (scene.nActiveEntities >= MAX_ENTITIES)
				continue;

			auto randf = []() { return float(rand()) / float(RAND_MAX); };

			// You need to spawn on a window edge
			Vec3f randomLocation;
			Vec3f randomVelocity;
			switch (rand() % 4)
			{
				case 0:
					randomLocation = Vec3f(0.0f, float(rand() % int(GameRenderer::GetHeight())), 0.0f);
					randomVelocity = Vec3f(randf(), randf() * 2.0f - 1.0f, 0.0f); break;
				case 1:
					randomLocation = Vec3f(GameRenderer::GetWidth(), float(rand() % int(GameRenderer::GetHeight())), 0.0f);
					randomVelocity = Vec3f(-randf(), randf() * 2.0f - 1.0f, 0.0f); break;
				case 2:
					randomLocation = Vec3f(float(rand() % int(GameRenderer::GetWidth())), 0.0f, 0.0f);
					randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf(), 0.0f); break;
				case 3:
					randomLocation = Vec3f(float(rand() % int(GameRenderer::GetWidth())), GameRenderer::GetHeight(), 0.0f);
					randomVelocity = Vec3f(randf() * 2.0f - 1.0f, -randf(), 0.0f); break;
				default:
					break;
			}

			EntityID asteroid = scene.NewEntity("Asteroid");
			scene.Assign<CCollidable>(asteroid);
			CTransform* pTranform = scene.Assign<CTransform>(asteroid);
			pTranform->localPos = randomLocation;
			pTranform->localSca = Vec3f(90.0f, 90.0f, 1.0f);
			pTranform->localRot = randf() * 6.282f;
			
			scene.Assign<CDynamics>(asteroid)->vel = randomVelocity * 60.0f;

			scene.Assign<CVisibility>(asteroid);
			scene.Assign<CAsteroid>(asteroid)->type = AsteroidType(rand() % 4);
		}
	}
}

void CollisionSystemUpdate(Scene& scene, float /* deltaTime */)
{
	PROFILE();

	bool bContinueOuter = false;
	for (EntityID asteroid : SceneIterator<CAsteroid>(scene))
	{
		float asteroidRad = scene.Get<CCollidable>(asteroid)->radius;

		// CTransform* pTrans = scene.Get<CTransform>(asteroid);
		// DebugDraw::Draw2DCircle(scene, Vec2f(pTrans->localPos.x, pTrans->localPos.y), asteroidRad + 10.0f, Vec3f(1.0f, 0.0f, 0.0f));

		for (EntityID bullet : SceneIterator<CBullet>(scene))
		{
			float bulletRad = scene.Get<CCollidable>(bullet)->radius;

			float distance = (scene.Get<CTransform>(asteroid)->localPos - scene.Get<CTransform>(bullet)->localPos).GetLength();
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
			float distance = (scene.Get<CTransform>(asteroid)->localPos - scene.Get<CTransform>(PLAYER_ID)->localPos).GetLength();
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
			CVisibility* visiblity = scene.Get<CVisibility>(PLAYER_ID);
			visiblity->visible = true;
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

	for (EntityID id : SceneIterator<CTransform, CDynamics>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CDynamics* pDynamics = scene.Get<CDynamics>(id);

		pDynamics->vel = pDynamics->vel + pDynamics->accel * deltaTime;
		pTransform->localPos = pTransform->localPos + pDynamics->vel * deltaTime;

		if (pTransform->localPos.x < 0.0f)
		{
			pTransform->localPos.x = GameRenderer::GetWidth();
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->localPos.x > GameRenderer::GetWidth())
		{
			pTransform->localPos.x = 0.0f;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}

		if (pTransform->localPos.y < 0.0f)
		{
			pTransform->localPos.y = GameRenderer::GetHeight();
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
		else if (pTransform->localPos.y > GameRenderer::GetHeight())
		{
			pTransform->localPos.y = 0.0f;
			if (scene.Has<CBullet>(id)) scene.DestroyEntity(id);
		}
	}
}

void ShipControlSystemUpdate(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneIterator<CTransform, CPlayerControl, CDynamics>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CDynamics* pDynamics = scene.Get<CDynamics>(id);
		CPlayerControl* pControl = scene.Get<CPlayerControl>(id);

		if (Input::GetKeyDown(SDL_SCANCODE_ESCAPE))
		{
			LoadMenu();
		}

		// If player isn't visible we consider them dead
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
			accel.x = cosf(pTransform->localRot.z);
			accel.y = sinf(pTransform->localRot.z);
			accel = accel * -pControl->thrust;
		}

		if (Input::GetKeyDown(SDL_SCANCODE_UP))
			AudioDevice::UnPauseSound(pControl->enginePlayingSound);
		else if (Input::GetKeyUp(SDL_SCANCODE_UP))
			AudioDevice::PauseSound(pControl->enginePlayingSound);

		pDynamics->accel = accel - pDynamics->vel * pControl->dampening;

		if (Input::GetKeyHeld(SDL_SCANCODE_LEFT))
			pTransform->localRot += pControl->rotateSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_RIGHT))
			pTransform->localRot -= pControl->rotateSpeed * deltaTime;

		// Shoot a bullet
		if (Input::GetKeyDown(SDL_SCANCODE_SPACE))
		{
			SpawnBullet(scene, pTransform, pDynamics->vel);
			AudioDevice::PlaySound(scene.Get<CSounds>(PLAYER_ID)->shootSound, 1.0f, false);
		}
	}
}

void MenuInterationSystem(Scene& scene, float /* deltaTime */)
{
	PROFILE();

	for (EntityID id : SceneIterator<CTransform, CMenuInteraction>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CMenuInteraction* pInteraction = scene.Get<CMenuInteraction>(id);

		eastl::vector<Vec2f> points;
		points.push_back(Vec2f(0.f, 0.0f));
		points.push_back(Vec2f(0.7f, 0.5f));
		points.push_back(Vec2f(0.f, 1.f));

		Matrixf pivotAdjust = Matrixf::MakeTranslation(Vec3f(-0.5f, -0.5f, 0.0f));
		Matrixf world = Matrixf::MakeTRS(pTransform->localPos, Vec3f(0.0f, 0.0f, pTransform->localRot.z), pTransform->localSca) * pivotAdjust;

		GfxDraw::Paint paint;
		paint.drawStyle = GfxDraw::DrawStyle::Stroke;
		paint.strokeThickness = 0.07f;
		paint.strokeColor = Vec4f(1.0f);
		GfxDraw::SetTransform(world);
		GfxDraw::SetGeometryMode(GfxDraw::GeometryMode::ZAlign);
		GfxDraw::Polyshape(points, paint);
		GfxDraw::SetTransform(Matrixf::Identity());

		const float w = GameRenderer::GetWidth();
		const float h = GameRenderer::GetHeight();
		float validPositions[] = { h / 2.0f + 18.0f, h / 2.0f - 62.0f};

		if (Input::GetKeyDown(SDL_SCANCODE_UP) && pInteraction->currentState == CMenuInteraction::Quit)
		{
			pInteraction->currentState = CMenuInteraction::Start;
		}
		if (Input::GetKeyDown(SDL_SCANCODE_DOWN) && pInteraction->currentState == CMenuInteraction::Start)
		{
			pInteraction->currentState = CMenuInteraction::Quit;
		}
		pTransform->localPos.y = validPositions[pInteraction->currentState];

		if (Input::GetKeyDown(SDL_SCANCODE_RETURN))
		{
			switch (pInteraction->currentState)
			{
			case CMenuInteraction::Start:
				LoadMainScene();
				break;
			case CMenuInteraction::Quit:
				Engine::StartShutdown();
				break;
			default:
				break;
			}
		}
	}
}

void OnPlayerControlRemoved(Scene& scene, EntityID entity)
{
	CPlayerControl* pPlayerControl = scene.Get<CPlayerControl>(entity);

	AudioDevice::StopSound(pPlayerControl->enginePlayingSound);
}