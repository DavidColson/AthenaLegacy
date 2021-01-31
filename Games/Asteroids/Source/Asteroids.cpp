#include "Asteroids.h"
#include "Systems.h"
#include "Components.h"

#include <Vec4.h>
#include <Matrix.h>
#include <Editor/Editor.h>
#include <Scene.h>
#include <Engine.h>
#include <Input/Input.h>
#include <Profiler.h>
#include <Rendering/FontSystem.h>
#include <Rendering/ParticlesSystem.h>
#include <Rendering/PostProcessingSystem.h>
#include <Rendering/GameRenderer.h>
#include <TypeSystem.h>
#include <AudioDevice.h>

#include <Variant.h>

#include <SDL.h>

namespace
{
	GfxDraw::PolyshapeMesh asteroidMeshCache[4];
	GfxDraw::PolyshapeMesh playerMeshCache;
}

GfxDraw::PolyshapeMesh& GetAsteroidMesh(AsteroidType type)
{
	return asteroidMeshCache[(uint8_t)type];
}

GfxDraw::PolyshapeMesh& GetPlayerMesh()
{
	return playerMeshCache;
}

Scene* CreateMainAsteroidsScene()
{
	Scene& scene = *(new Scene());

	scene.RegisterSystem(SystemPhase::Update, ShipControlSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, MovementSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, CollisionSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, AsteroidSpawning);
	scene.RegisterSystem(SystemPhase::Update, InvincibilitySystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, DrawAsteroids);
	scene.RegisterSystem(SystemPhase::Update, DrawShips);
	scene.RegisterSystem(SystemPhase::Update, DrawBullets);

	scene.RegisterReactiveSystem<CPlayerControl>(Reaction::OnRemove, OnPlayerControlRemoved);

	srand(unsigned int(time(nullptr)));
	auto randf = []() { return float(rand()) / float(RAND_MAX); };

	// Create the ship
	Vec2f verts[5] = {
		Vec2f(0.f, 0.5f),
		Vec2f(1.f, 0.8f),
		Vec2f(0.9f, 0.7f),
		Vec2f(0.9f, 0.3f),
		Vec2f(1.0f, 0.2f)
	};

	EntityID ship = scene.NewEntity("Player Ship");
	ASSERT(ship == PLAYER_ID, "Player must be spawned first");
	CTransform* pShipTransform = scene.Assign<CTransform>(ship);

	const float w = GameRenderer::GetWidth();
	const float h = GameRenderer::GetHeight();
	pShipTransform->localPos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	pShipTransform->localSca = Vec3f(30.f, 35.f, 1.0f);
	scene.Assign<CDynamics>(ship);

	CPlayerControl* pPlayer = scene.Assign<CPlayerControl>(ship);
	scene.Assign<CCollidable>(ship)->radius = 17.f;
	scene.Assign<CVisibility>(ship);
	CPlayerUI* pPlayerUI = scene.Assign<CPlayerUI>(ship);
	scene.Assign<CPostProcessing>(ship);
	scene.Assign<CInvincibility>(ship);
	scene.Assign<CShipDraw>(ship);

	CSounds* pSounds = scene.Assign<CSounds>(ship);
	pSounds->engineSound = AssetHandle("Audio/Engine.wav");
	pSounds->shootSound = AssetHandle("Audio/Shoot.wav");
	pSounds->explosionSound = AssetHandle("Audio/Explosion.wav");

	pPlayer->enginePlayingSound = AudioDevice::PlaySound(pSounds->engineSound, 0.3f, true);
	AudioDevice::PauseSound(pPlayer->enginePlayingSound);

	// Create some asteroids
	for (int i = 0; i < 10; i++)
	{
		Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
		Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
		float randomRotation = randf() * 6.282f;
		EntityID asteroid = scene.NewEntity("Asteroid");
		scene.Assign<CCollidable>(asteroid);
		CTransform* pTranform = scene.Assign<CTransform>(asteroid);
		pTranform->localPos = randomLocation;
		pTranform->localSca = Vec3f(90.0f, 90.0f, 1.0f);
		pTranform->localRot = randomRotation;

		scene.Assign<CDynamics>(asteroid)->vel = randomVelocity;

		scene.Assign<CVisibility>(asteroid);
		scene.Assign<CAsteroid>(asteroid)->type = AsteroidType(rand() % 4);
	}

	scene.Assign<CAsteroidSpawner>(ship);

	// Create the lives
	float offset = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		EntityID life = scene.NewEntity("Life");
		
		CTransform* pTransform = scene.Assign<CTransform>(life);
		pTransform->localPos = Vec3f(150.f + offset, h - 85.0f, 0.0f);
		pTransform->localSca = Vec3f(30.f, 35.f, 1.0f);
		pTransform->localRot = -3.14159f / 2.0f;
		offset += 30.0f;
		
		scene.Assign<CShipDraw>(life);
		scene.Assign<CVisibility>(life);
		pPlayer->lifeEntities[i] = life;
	}


	// Create score counters
	{
		EntityID currentScoreEnt =scene.NewEntity("Current Score");
		pPlayerUI->currentScoreEntity = currentScoreEnt;
		scene.Assign<CTransform>(currentScoreEnt)->localPos = Vec3f(150.0f, h - 53.0f, 0.0f);
		scene.Assign<CPlayerScore>(currentScoreEnt);
		CText* pCurrScoreText = scene.Assign<CText>(currentScoreEnt);
		pCurrScoreText->text = "0";
		pCurrScoreText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

		EntityID highScoreEnt = scene.NewEntity("High Score");
		scene.Assign<CTransform>(highScoreEnt)->localPos = Vec3f(w - 150.f, h - 53.0f, 0.0f);
		CText* pHiScoreText = scene.Assign<CText>(highScoreEnt);
		pHiScoreText->text = "0";
		pHiScoreText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
	}

	// Create game over text
	{
		EntityID gameOver = scene.NewEntity("Game Over");
		pPlayerUI->gameOverEntity = gameOver;
		scene.Assign<CTransform>(gameOver)->localPos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
		scene.Assign<CGameOver>(gameOver);
		scene.Assign<CVisibility>(gameOver)->visible = false;
		CText* pText = scene.Assign<CText>(gameOver);
		pText->text = "Game Over";
		pText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
	}
	return &scene;
}

Scene* CreateMainMenuScene()
{
	Scene& scene = *(new Scene());
	scene.RegisterSystem(SystemPhase::Update, DrawAsteroids);
	scene.RegisterSystem(SystemPhase::Update, MenuInterationSystem);

	const float w = GameRenderer::GetWidth();
	const float h = GameRenderer::GetHeight();

	EntityID titleText =scene.NewEntity("Main Menu Title");
	CTransform* pTrans = scene.Assign<CTransform>(titleText);
	pTrans->localPos = Vec3f(w / 2.0f, h / 2.0f + 200.0f, 0.0f);
	pTrans->localSca = Vec3f(2.0f, 2.0f, 2.0f);
	scene.Assign<CPostProcessing>(titleText);
	CText* pTitleText = scene.Assign<CText>(titleText);
	pTitleText->text = "Asteroids!";
	pTitleText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

	EntityID startOption =scene.NewEntity("Start Option");
	scene.Assign<CTransform>(startOption)->localPos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	CText* pStartText = scene.Assign<CText>(startOption);
	pStartText->text = "Start";
	pStartText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

	EntityID quitOption = scene.NewEntity("Quit Option");
	scene.Assign<CTransform>(quitOption)->localPos = Vec3f(w / 2.0f, h / 2.0f - 80.0f, 0.0f);
	CText* pQuitText = scene.Assign<CText>(quitOption);
	pQuitText->text = "Quit";
	pQuitText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

	EntityID buttonSelector =scene.NewEntity("Button Selector");
	scene.Assign<CVisibility>(buttonSelector);
	scene.Assign<CMenuInteraction>(buttonSelector);
	CTransform* pTransform = scene.Assign<CTransform>(buttonSelector);
	pTransform->localPos = Vec3f(w / 2.0f - 100.0f, h / 2.0f + 18.0f, 0.0f);
	pTransform->localSca = Vec3f(30.f, 30.0f, 1.0f);

	return &scene;
}

void LoadMainScene()
{
	Engine::SetActiveScene(CreateMainAsteroidsScene());
}

void LoadMenu()
{
	Engine::SetActiveScene(CreateMainMenuScene());
}

int main(int argc, char *argv[])
{
	Engine::Initialize("Games/Asteroids/Asteroids.cfg");

	// Make some asteroids!
	GfxDraw::Paint asteroidPaint;
	asteroidPaint.drawStyle = GfxDraw::DrawStyle::Stroke;
	asteroidPaint.strokeThickness = 0.04f;
	asteroidPaint.strokeColor = Vec4f(1.0f);
	{
		eastl::vector<Vec2f> asteroidPoly;
		asteroidPoly.push_back(Vec2f(0.056f, 0.265f));
		asteroidPoly.push_back(Vec2f(0.312f, 0.074f));
		asteroidPoly.push_back(Vec2f(0.683f, 0.086f));
		asteroidPoly.push_back(Vec2f(0.943f, 0.298f));
		asteroidPoly.push_back(Vec2f(0.974f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.83f, 0.85f));
		asteroidPoly.push_back(Vec2f(0.64f, 0.75f));
		asteroidPoly.push_back(Vec2f(0.673f, 0.952f));
		asteroidPoly.push_back(Vec2f(0.348f, 0.96f));
		asteroidPoly.push_back(Vec2f(0.37f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.213f, 0.78f));
		asteroidPoly.push_back(Vec2f(0.05f, 0.54f));
		asteroidMeshCache[0] = GfxDraw::CreatePolyshape(asteroidPoly, asteroidPaint);
	}
	{
		eastl::vector<Vec2f> asteroidPoly;
		asteroidPoly.push_back(Vec2f(0.03f, 0.379f));
		asteroidPoly.push_back(Vec2f(0.03f, 0.64f));
		asteroidPoly.push_back(Vec2f(0.314f, 0.69f));
		asteroidPoly.push_back(Vec2f(0.348f, 0.96f));
		asteroidPoly.push_back(Vec2f(0.673f, 0.952f));
		asteroidPoly.push_back(Vec2f(0.698f, 0.724f));
		asteroidPoly.push_back(Vec2f(0.97f, 0.645f));
		asteroidPoly.push_back(Vec2f(0.936f, 0.228f));
		asteroidPoly.push_back(Vec2f(0.555f, 0.028f));
		asteroidPoly.push_back(Vec2f(0.22f, 0.123f));
		asteroidMeshCache[1] = GfxDraw::CreatePolyshape(asteroidPoly, asteroidPaint);
	}
	{
		eastl::vector<Vec2f> asteroidPoly;
		asteroidPoly.push_back(Vec2f(0.05f, 0.54f));
		asteroidPoly.push_back(Vec2f(0.213f, 0.78f));
		asteroidPoly.push_back(Vec2f(0.37f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.348f, 0.96f));
		asteroidPoly.push_back(Vec2f(0.673f, 0.952f));
		asteroidPoly.push_back(Vec2f(0.64f, 0.75f));
		asteroidPoly.push_back(Vec2f(0.83f, 0.85f));
		asteroidPoly.push_back(Vec2f(0.974f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.943f, 0.298f));
		asteroidPoly.push_back(Vec2f(0.683f, 0.086f));
		asteroidPoly.push_back(Vec2f(0.312f, 0.074f));
		asteroidPoly.push_back(Vec2f(0.056f, 0.265f));
		asteroidMeshCache[2] = GfxDraw::CreatePolyshape(asteroidPoly, asteroidPaint);
	}
	{
		eastl::vector<Vec2f> asteroidPoly;
		asteroidPoly.push_back(Vec2f(0.066f, 0.335f));
		asteroidPoly.push_back(Vec2f(0.077f, 0.683f));
		asteroidPoly.push_back(Vec2f(0.3f, 0.762f));
		asteroidPoly.push_back(Vec2f(0.348f, 0.96f));
		asteroidPoly.push_back(Vec2f(0.673f, 0.952f));
		asteroidPoly.push_back(Vec2f(0.724f, 0.752f));
		asteroidPoly.push_back(Vec2f(0.967f, 0.63f));
		asteroidPoly.push_back(Vec2f(0.946f, 0.312f));
		asteroidPoly.push_back(Vec2f(0.706f, 0.353f));
		asteroidPoly.push_back(Vec2f(0.767f, 0.07f));
		asteroidPoly.push_back(Vec2f(0.37f, 0.07f));
		asteroidPoly.push_back(Vec2f(0.21f, 0.33f));
		asteroidMeshCache[3] = GfxDraw::CreatePolyshape(asteroidPoly, asteroidPaint);
	}

	{
		GfxDraw::Paint paint;
		paint.drawStyle = GfxDraw::DrawStyle::Stroke;
		paint.strokeThickness = 0.07f;
		paint.strokeColor = Vec4f(1.0f);
		eastl::vector<Vec2f> playerPoly;
		playerPoly.push_back(Vec2f(0.f, 0.5f));
		playerPoly.push_back(Vec2f(1.f, 0.8f));
		playerPoly.push_back(Vec2f(0.9f, 0.7f));
		playerPoly.push_back(Vec2f(0.9f, 0.3f));
		playerPoly.push_back(Vec2f(1.0f, 0.2f));
		playerMeshCache = GfxDraw::CreatePolyshape(playerPoly, paint);
	}

	// Run everything
	Engine::Run(CreateMainMenuScene());

	return 0;
}