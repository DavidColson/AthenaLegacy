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

#include <Variant.h>

#include <SDL.h>

struct Component
{
  int myInt{ 5 };
  int mySecondInt{ 3 };

  float myFloat{ 21.412312731f };
  double myDouble{ 21.4123127311928746123 };

  eastl::string myString{ "Ducks" };
  bool myBool = false;

  REFLECT()
};

REFLECT_BEGIN(Component)
REFLECT_MEMBER(myInt)
REFLECT_MEMBER(mySecondInt)
REFLECT_MEMBER(myFloat)
REFLECT_MEMBER(myDouble)
REFLECT_MEMBER(myString)
REFLECT_MEMBER(myBool)
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
	pShipTransform->localPos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	pShipTransform->localSca = Vec3f(30.f, 35.f, 1.0f);
	scene.Assign<CDynamics>(ship);

	CPlayerControl* pPlayer = scene.Assign<CPlayerControl>(ship);
	scene.Assign<CCollidable>(ship)->radius = 17.f;
	scene.Assign<CVisibility>(ship);
	CPlayerUI* pPlayerUI = scene.Assign<CPlayerUI>(ship);
	scene.Assign<CPostProcessing>(ship);
	scene.Assign<CInvincibility>(ship);
	scene.Assign<CPolyShape>(ship)->points.assign(verts, verts + 5);

	CSounds* pSounds = scene.Assign<CSounds>(ship);
	pSounds->engineSound = AssetHandle("Resources/Audio/Engine.wav");
	pSounds->shootSound = AssetHandle("Resources/Audio/Shoot.wav");
	pSounds->explosionSound = AssetHandle("Resources/Audio/Explosion.wav");

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
		pTransform->localPos = Vec3f(150.f + offset, h - 85.0f, 0.0f);
		pTransform->localSca = Vec3f(30.f, 35.f, 1.0f);
		pTransform->localRot = -3.14159f / 2.0f;
		offset += 30.0f;
		
		scene.Assign<CPolyShape>(life)->points.assign(verts, verts + 5);
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
		pCurrScoreText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");

		EntityID highScoreEnt = scene.NewEntity("High Score");
		scene.Assign<CTransform>(highScoreEnt)->localPos = Vec3f(w - 150.f, h - 53.0f, 0.0f);
		CText* pHiScoreText = scene.Assign<CText>(highScoreEnt);
		pHiScoreText->text = "0";
		pHiScoreText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");
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
		pText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");
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
	CTransform* pTrans = scene.Assign<CTransform>(titleText);
	pTrans->localPos = Vec3f(w / 2.0f, h / 2.0f + 200.0f, 0.0f);
	pTrans->localSca = Vec3f(2.0f, 2.0f, 2.0f);
	scene.Assign<CPostProcessing>(titleText);
	CText* pTitleText = scene.Assign<CText>(titleText);
	pTitleText->text = "Asteroids!";
	pTitleText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");

	EntityID startOption =scene.NewEntity("Start Option");
	scene.Assign<CTransform>(startOption)->localPos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	CText* pStartText = scene.Assign<CText>(startOption);
	pStartText->text = "Start";
	pStartText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");

	EntityID quitOption = scene.NewEntity("Quit Option");
	scene.Assign<CTransform>(quitOption)->localPos = Vec3f(w / 2.0f, h / 2.0f - 80.0f, 0.0f);
	CText* pQuitText = scene.Assign<CText>(quitOption);
	pQuitText->text = "Quit";
	pQuitText->fontAsset = AssetHandle("Resources/Fonts/Hyperspace/Hyperspace Bold.otf");

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
	// Unused at the moment
	(void)argc;
	(void)argv;


	Engine::Initialize();

	// Type system testing
	{
		// Variant newComponent = TypeDatabase::CreateNew("Component");
		Variant newComponent = TypeDatabase::GetFromString("Component").New();
		Component& newComp = newComponent.GetValue<Component>();

		Log::Debug("newComp int %i", newComp.myInt);

		TypeData& typeData = TypeDatabase::Get<Component>();
		Member& myInMember = typeData.GetMember("myInt");
		myInMember.Set(&newComp, 1337);

		Log::Debug("newComp int %i", newComp.myInt);

		Log::Debug("Iterator printing Members of type: %s", typeData.name);
		for (Member& member : typeData)
		{			
			Log::Debug("Name: %s Type: %s val: %i", member.name, member.GetType().name, *member.GetAs<int>(&newComp));
		}
	}

	// Run everything
	Engine::Run(CreateMainMenuScene());

	return 0;
}