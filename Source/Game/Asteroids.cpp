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
#include <Imgui/imgui.h>
#include <TypeSystem.h>
#include <AudioDevice.h>

#include <functional>
#include <time.h>

struct Component
{
  int myInt{ 5 };
  int mySecondInt{ 3 };

  REFLECT()
};

REFLECT_BEGIN(Component)
REFLECT_MEMBER(myInt)
REFLECT_MEMBER(mySecondInt)
REFLECT_END()

Scene* CreateMainAsteroidsScene()
{
	Scene& scene = *(new Scene());

	scene.RegisterSystem(SystemPhase::Update, ShipControlSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, MovementSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, CollisionSystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, AsteroidSpawning);
	scene.RegisterSystem(SystemPhase::Update, InvincibilitySystemUpdate);
	scene.RegisterSystem(SystemPhase::Update, DrawPolyShapes);

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

	const float w = GfxDevice::GetWindowWidth();
	const float h = GfxDevice::GetWindowHeight();
	pShipTransform->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	pShipTransform->sca = Vec3f(30.f, 35.f, 1.0f);

	CPlayerControl* pPlayer = scene.Assign<CPlayerControl>(ship);
	scene.Assign<CCollidable>(ship)->radius = 17.f;
	scene.Assign<CVisibility>(ship);
	CPlayerUI* pPlayerUI = scene.Assign<CPlayerUI>(ship);
	scene.Assign<CPostProcessing>(ship);
	scene.Assign<CInvincibility>(ship);
	scene.Assign<CPolyShape>(ship)->points.assign(verts, verts + 5);

	CSounds* pSounds = scene.Assign<CSounds>(ship);
	pSounds->engineSound = AudioDevice::LoadSound("Resources/Audio/Engine.wav");
	pSounds->shootSound = AudioDevice::LoadSound("Resources/Audio/Shoot.wav");
	pSounds->explosionSound = AudioDevice::LoadSound("Resources/Audio/Explosion.wav");

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
		pTranform->pos = randomLocation;
		pTranform->sca = Vec3f(90.0f, 90.0f, 1.0f);
		pTranform->vel = randomVelocity;
		pTranform->rot = randomRotation;

		scene.Assign<CVisibility>(asteroid);
		scene.Assign<CAsteroid>(asteroid);
		scene.Assign<CPolyShape>(asteroid)->points = GetRandomAsteroidMesh();
	}

	scene.Assign<CAsteroidSpawner>(ship);

	// Create the lives
	float offset = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		EntityID life = scene.NewEntity("Life");
		
		CTransform* pTransform = scene.Assign<CTransform>(life);
		pTransform->pos = Vec3f(150.f + offset, h - 85.0f, 0.0f);
		pTransform->sca = Vec3f(30.f, 35.f, 1.0f);
		pTransform->rot = -3.14159f / 2.0f;
		offset += 30.0f;
		
		scene.Assign<CPolyShape>(life)->points.assign(verts, verts + 5);
		scene.Assign<CVisibility>(life);
		pPlayer->lifeEntities[i] = life;
	}


	// Create score counters
	{
		EntityID currentScoreEnt =scene.NewEntity("Current Score");
		pPlayerUI->currentScoreEntity = currentScoreEnt;
		scene.Assign<CText>(currentScoreEnt)->text = "0";
		scene.Assign<CTransform>(currentScoreEnt)->pos = Vec3f(150.0f, h - 53.0f, 0.0f);
		scene.Assign<CPlayerScore>(currentScoreEnt);

		EntityID highScoreEnt = scene.NewEntity("High Score");
		scene.Assign<CText>(highScoreEnt)->text = "0";
		scene.Assign<CTransform>(highScoreEnt)->pos = Vec3f(w - 150.f, h - 53.0f, 0.0f);
	}

	// Create game over text
	{
		EntityID gameOver = scene.NewEntity("Game Over");
		pPlayerUI->gameOverEntity = gameOver;
		scene.Assign<CTransform>(gameOver)->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
		scene.Assign<CGameOver>(gameOver);
		scene.Assign<CVisibility>(gameOver)->visible = false;
		CText* pText = scene.Assign<CText>(gameOver);
		pText->text = "Game Over";
	}
	return &scene;
}

Scene* CreateMainMenuScene()
{
	Scene& scene = *(new Scene());
	scene.RegisterSystem(SystemPhase::Update, DrawPolyShapes);
	scene.RegisterSystem(SystemPhase::Update, MenuInterationSystem);

	const float w = GfxDevice::GetWindowWidth();
	const float h = GfxDevice::GetWindowHeight();

	EntityID titleText =scene.NewEntity("Main Menu Title");
	scene.Assign<CText>(titleText)->text = "Asteroids!";
	CTransform* pTrans = scene.Assign<CTransform>(titleText);
	pTrans->pos = Vec3f(w / 2.0f, h / 2.0f + 200.0f, 0.0f);
	pTrans->sca = Vec3f(2.0f, 2.0f, 2.0f);
	scene.Assign<CPostProcessing>(titleText);

	EntityID startOption =scene.NewEntity("Start Option");
	scene.Assign<CText>(startOption)->text = "Start";
	scene.Assign<CTransform>(startOption)->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);

	EntityID quitOption =scene.NewEntity("Quit Option");
	scene.Assign<CText>(quitOption)->text = "Quit";
	scene.Assign<CTransform>(quitOption)->pos = Vec3f(w / 2.0f, h / 2.0f - 80.0f, 0.0f);

	EntityID buttonSelector =scene.NewEntity("Button Selector");
	scene.Assign<CVisibility>(buttonSelector);
	scene.Assign<CMenuInteraction>(buttonSelector);

	Vec2f verts[3] = {
		Vec2f(0.f, 0.0f),
		Vec2f(0.7f, 0.5f),
		Vec2f(0.f, 1.f)
	};
	scene.Assign<CPolyShape>(buttonSelector)->points.assign(verts, verts + 3);
	CTransform* pTransform = scene.Assign<CTransform>(buttonSelector);
	pTransform->pos = Vec3f(w / 2.0f - 100.0f, h / 2.0f + 18.0f, 0.0f);
	pTransform->sca = Vec3f(30.f, 30.0f, 1.0f);

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
	// Unused at the moment
	(void)argc;
	(void)argv;


	Engine::Initialize();

	// Type system testing
	{
		Component testComponent;

		TypeData& typeData = TypeDatabase::Get<Component>();
		Member& myInMember = typeData.GetMember("myInt");

		myInMember.Set(&testComponent, 1337);

		Log::Debug("Iterator printing Members of type: %s", typeData.name);
		for (Member& member : typeData)
		{			
			Log::Debug("Name: %s Type: %s val: %i", member.name, member.GetType().name, *member.Get<int>(&testComponent));
		}
	}

	// Run everything
	Engine::Run(CreateMainMenuScene());

	return 0;
}