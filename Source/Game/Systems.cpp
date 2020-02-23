#include "Systems.h"

#include "Asteroids.h"
#include "Components.h"

#include <AudioDevice.h>
#include <Rendering/DebugDraw.h>
#include <Rendering/FontSystem.h>
#include <Rendering/ParticlesSystem.h>
#include <Rendering/ShapesSystem.h>
#include <Input/Input.h>
#include <Utility.h>
#include <Profiler.h>
#include <Vec4.h>


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
	pBulletTrans->sca = Vec3f(7.0f);

	scene.Assign<CVisibility>(bullet);
	scene.Assign<CCollidable>(bullet)->radius = 4.0f;
	
	Vec2f verts[2] = {
		Vec2f(0.f, 0.0f),
		Vec2f(0.f, 1.0f)
	};
	CPolyShape* pPolyShape = scene.Assign<CPolyShape>(bullet);
	pPolyShape->points.assign(verts, verts + 2);
	pPolyShape->thickness = 17.0f;
	pPolyShape->connected = false;
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

		int mesh = rand() % 4;
		scene.Assign<CVisibility>(newAsteroid);
		scene.Assign<CAsteroid>(newAsteroid)->hitCount = scene.Get<CAsteroid>(asteroid)->hitCount + 1;
		scene.Assign<CPolyShape>(newAsteroid)->points = GetRandomAsteroidMesh();
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

std::vector<Vec2f> GetRandomAsteroidMesh()
{
	static Vec2f asteroidMesh1[] = {
		Vec2f(0.03f, 0.379f),
		Vec2f(0.03f, 0.64f),
		Vec2f(0.314f, 0.69f),
		Vec2f(0.348f, 0.96f),
		Vec2f(0.673f, 0.952f),
		Vec2f(0.698f, 0.724f),
		Vec2f(0.97f, 0.645f),
		Vec2f(0.936f, 0.228f),
		Vec2f(0.555f, 0.028f),
		Vec2f(0.22f, 0.123f)
	};
	static Vec2f asteroidMesh2[] = {
		Vec2f(0.05f, 0.54f),
		Vec2f(0.213f, 0.78f),
		Vec2f(0.37f, 0.65f),
		Vec2f(0.348f, 0.96f),
		Vec2f(0.673f, 0.952f),
		Vec2f(0.64f, 0.75f),
		Vec2f(0.83f, 0.85f),
		Vec2f(0.974f, 0.65f),
		Vec2f(0.943f, 0.298f),
		Vec2f(0.683f, 0.086f),
		Vec2f(0.312f, 0.074f),
		Vec2f(0.056f, 0.265f)
	};
	static Vec2f asteroidMesh3[] = {
		Vec2f(0.066f, 0.335f),
		Vec2f(0.077f, 0.683f),
		Vec2f(0.3f, 0.762f),
		Vec2f(0.348f, 0.96f),
		Vec2f(0.673f, 0.952f),
		Vec2f(0.724f, 0.752f),
		Vec2f(0.967f, 0.63f),
		Vec2f(0.946f, 0.312f),
		Vec2f(0.706f, 0.353f),
		Vec2f(0.767f, 0.07f),
		Vec2f(0.37f, 0.07f),
		Vec2f(0.21f, 0.33f)
	};
	static Vec2f asteroidMesh4[] = {
		Vec2f(0.056f, 0.284f),
		Vec2f(0.064f, 0.752f),
		Vec2f(0.353f, 0.762f),
		Vec2f(0.286f, 0.952f),
		Vec2f(0.72f, 0.944f),
		Vec2f(0.928f, 0.767f),
		Vec2f(0.962f, 0.604f),
		Vec2f(0.568f, 0.501f),
		Vec2f(0.967f, 0.366f),
		Vec2f(0.857f, 0.16f),
		Vec2f(0.563f, 0.217f),
		Vec2f(0.358f, 0.043f)
	};

	std::vector<Vec2f> vec;
	switch (rand() % 4)
	{
	case 0: vec.assign(asteroidMesh1, asteroidMesh1 + 10); break;
	case 1: vec.assign(asteroidMesh2, asteroidMesh2 + 12); break;
	case 2: vec.assign(asteroidMesh3, asteroidMesh3 + 12); break;
	case 3: vec.assign(asteroidMesh4, asteroidMesh4 + 12); break;
	default: break;
	}
	return vec;
}

// **********
// SYSTEMS
// **********

void DrawPolyShapes(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID shape : SceneView<CPolyShape, CTransform, CVisibility>(scene))
	{
		if (scene.Get<CVisibility>(shape)->visible == false)
			continue;

		CTransform* pTrans = scene.Get<CTransform>(shape);
		Matrixf posMat = Matrixf::Translate(pTrans->pos);
		Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, pTrans->rot));
		Matrixf scaMat = Matrixf::Scale(pTrans->sca);
		Matrixf pivotAdjust = Matrixf::Translate(Vec3f(-0.5f, -0.5f, 0.0f));
		Matrixf world = posMat * rotMat * scaMat * pivotAdjust;

		// TODO: Lots of memory thrashing here, do better
		std::vector<Vec2f> transformedVerts;
		for (const Vec2f& vert : scene.Get<CPolyShape>(shape)->points)
			transformedVerts.push_back(Vec2f::Project4D(world * Vec4f::Embed2D(vert)));

		Shapes::DrawPolyLine(scene, transformedVerts, scene.Get<CPolyShape>(shape)->thickness, Vec3f(1.0f, 1.0f, 1.0f), scene.Get<CPolyShape>(shape)->connected);
	}	
}

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

		if (Input::GetKeyDown(SDL_SCANCODE_ESCAPE))
		{
			LoadMenu();
		}

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

void MenuInterationSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID id : SceneView<CTransform, CMenuInteraction>(scene))
	{
		CTransform* pTransform = scene.Get<CTransform>(id);
		CMenuInteraction* pInteraction = scene.Get<CMenuInteraction>(id);

		const float w = GfxDevice::GetWindowWidth();
		const float h = GfxDevice::GetWindowHeight();
		float validPositions[] = { h / 2.0f + 18.0f, h / 2.0f - 62.0f};

		if (Input::GetKeyDown(SDL_SCANCODE_UP) && pInteraction->currentState == CMenuInteraction::Quit)
		{
			pInteraction->currentState = CMenuInteraction::Start;
		}
		if (Input::GetKeyDown(SDL_SCANCODE_DOWN) && pInteraction->currentState == CMenuInteraction::Start)
		{
			pInteraction->currentState = CMenuInteraction::Quit;
		}
		pTransform->pos.y = validPositions[pInteraction->currentState];

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