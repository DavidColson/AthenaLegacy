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
#include <Renderer/Renderer.h>
#include <Renderer/RenderFont.h>
#include <Renderer/ParticlesSystem.h>
#include <Imgui/imgui.h>
#include <TypeSystem.h>
#include <AudioDevice.h>

#include <functional>
#include <time.h>

std::vector<Shape> Game::g_asteroidMeshes;
Shape Game::g_shipMesh;

Scene* pMainScene{ nullptr };
Scene* pMainMenuScene{ nullptr };

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
	scene.RegisterSystem(SystemPhase::Update, InvincibilitySystemUpdate);

	Shape asteroid1;
	asteroid1.vertices = {
			Vertex(Vec3f(0.03f, 0.379f, 0.0f)),
			Vertex(Vec3f(0.03f, 0.64f, 0.0f)),
			Vertex(Vec3f(0.314f, 0.69f, 0.0f)),
			Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
			Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
			Vertex(Vec3f(0.698f, 0.724f, 0.0f)),
			Vertex(Vec3f(0.97f, 0.645f, 0.0f)),
			Vertex(Vec3f(0.936f, 0.228f, 0.f)),
			Vertex(Vec3f(0.555f, 0.028f, 0.f)),
			Vertex(Vec3f(0.22f, 0.123f, 0.f))
		};
	asteroid1.indices = { 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1 };
	Game::g_asteroidMeshes.push_back(asteroid1);

	Shape asteroid2;
	asteroid2.vertices = {
			Vertex(Vec3f(0.05f, 0.54f, 0.0f)),
			Vertex(Vec3f(0.213f, 0.78f, 0.0f)),
			Vertex(Vec3f(0.37f, 0.65f, 0.0f)),
			Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
			Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
			Vertex(Vec3f(0.64f, 0.75f, 0.0f)),
			Vertex(Vec3f(0.83f, 0.85f, 0.0f)),
			Vertex(Vec3f(0.974f, 0.65f, 0.0f)),
			Vertex(Vec3f(0.943f, 0.298f, 0.f)),
			Vertex(Vec3f(0.683f, 0.086f, 0.f)),
			Vertex(Vec3f(0.312f, 0.074f, 0.f)),
			Vertex(Vec3f(0.056f, 0.265f, 0.f))
		};
	asteroid2.indices = { 10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 1 };
	Game::g_asteroidMeshes.push_back(asteroid2);

	Shape asteroid3;
	asteroid3.vertices = {
			Vertex(Vec3f(0.066f, 0.335f, 0.0f)),
			Vertex(Vec3f(0.077f, 0.683f, 0.0f)),
			Vertex(Vec3f(0.3f, 0.762f, 0.0f)),
			Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
			Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
			Vertex(Vec3f(0.724f, 0.752f, 0.0f)),
			Vertex(Vec3f(0.967f, 0.63f, 0.0f)),
			Vertex(Vec3f(0.946f, 0.312f, 0.0f)),
			Vertex(Vec3f(0.706f, 0.353f, 0.f)),
			Vertex(Vec3f(0.767f, 0.07f, 0.f)),
			Vertex(Vec3f(0.37f, 0.07f, 0.f)),
			Vertex(Vec3f(0.21f, 0.33f, 0.f))
		};
	asteroid3.indices = { 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1 };
	Game::g_asteroidMeshes.push_back(asteroid3);

	Shape asteroid4;
	asteroid4.vertices = {
			Vertex(Vec3f(0.056f, 0.284f, 0.0f)),
			Vertex(Vec3f(0.064f, 0.752f, 0.0f)),
			Vertex(Vec3f(0.353f, 0.762f, 0.0f)),
			Vertex(Vec3f(0.286f, 0.952f, 0.0f)),
			Vertex(Vec3f(0.72f, 0.944f, 0.0f)),
			Vertex(Vec3f(0.928f, 0.767f, 0.0f)),
			Vertex(Vec3f(0.962f, 0.604f, 0.0f)),
			Vertex(Vec3f(0.568f, 0.501f, 0.0f)),
			Vertex(Vec3f(0.967f, 0.366f, 0.f)),
			Vertex(Vec3f(0.857f, 0.16f, 0.f)),
			Vertex(Vec3f(0.563f, 0.217f, 0.f)),
			Vertex(Vec3f(0.358f, 0.043f, 0.f))
		};
	asteroid4.indices = { 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1 };
	Game::g_asteroidMeshes.push_back(asteroid4);

	Game::g_shipMesh.vertices = {
			Vertex(Vec3f(0.f, 0.5f, 0.f)),
			Vertex(Vec3f(1.f, 0.8f, 0.f)),
			Vertex(Vec3f(0.9f, 0.7f, 0.f)),
			Vertex(Vec3f(0.9f, 0.3f, 0.f)),
			Vertex(Vec3f(1.0f, 0.2f, 0.f))
		};
	Game::g_shipMesh.indices = { 4, 0, 1, 2, 3, 4, 0, 1 };

	srand(unsigned int(time(nullptr)));
	auto randf = []() { return float(rand()) / float(RAND_MAX); };

	// Create the ship
	EntityID ship = scene.NewEntity("Player Ship");
	ASSERT(ship == PLAYER_ID, "Player must be spawned first");
	CTransform* pTransform = scene.Assign<CTransform>(ship);

	const float w = GfxDevice::GetWindowWidth();
	const float h = GfxDevice::GetWindowHeight();
	pTransform->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	pTransform->sca = Vec3f(30.f, 35.f, 1.0f);

	CPlayerControl* pPlayer = scene.Assign<CPlayerControl>(ship);
	scene.Assign<CCollidable>(ship)->radius = 17.f;
	scene.Assign<CVisibility>(ship);
	CPlayerUI* pPlayerUI = scene.Assign<CPlayerUI>(ship);
	scene.Assign<CPostProcessing>(ship);
	scene.Assign<CInvincibility>(ship);
	CDrawable* pDrawable = scene.Assign<CDrawable>(ship);
	pDrawable->vertices = Game::g_shipMesh.vertices;
	pDrawable->indices = Game::g_shipMesh.indices;

	CSounds* pSounds = scene.Assign<CSounds>(ship);
	pSounds->engineSound = AudioDevice::LoadSound("Resources/Audio/Engine.wav");
	pSounds->shootSound = AudioDevice::LoadSound("Resources/Audio/Shoot.wav");
	pSounds->explosionSound = AudioDevice::LoadSound("Resources/Audio/Explosion.wav");

	pPlayer->enginePlayingSound = AudioDevice::PlaySound(pSounds->engineSound, 0.3f, true);
	AudioDevice::PauseSound(pPlayer->enginePlayingSound);

	// Create some asteroids
	for (int i = 0; i < 15; i++)
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
		CDrawable* pDrawable = scene.Assign<CDrawable>(asteroid);
		int mesh = rand() % 4;
		pDrawable->vertices = Game::g_asteroidMeshes[mesh].vertices;
		pDrawable->indices = Game::g_asteroidMeshes[mesh].indices;
	}

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
		
		CDrawable* pDrawable = scene.Assign<CDrawable>(life);
		pDrawable->vertices = Game::g_shipMesh.vertices;
		pDrawable->indices = Game::g_shipMesh.indices;
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

	const float w = GfxDevice::GetWindowWidth();
	const float h = GfxDevice::GetWindowHeight();

	EntityID titleText =scene.NewEntity("Main Menu Title");
	scene.Assign<CText>(titleText)->text = "Asteroids!";
	scene.Assign<CTransform>(titleText)->pos = Vec3f(w / 2.0f, h / 2.0f + 200.0f, 0.0f);
	scene.Assign<CPostProcessing>(titleText);

	EntityID startOption =scene.NewEntity("Start Option");
	scene.Assign<CText>(startOption)->text = "Start";
	scene.Assign<CTransform>(startOption)->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
	scene.Assign<CPostProcessing>(startOption);

	EntityID quitOption =scene.NewEntity("Quit Option");
	scene.Assign<CText>(quitOption)->text = "Quit";
	scene.Assign<CTransform>(quitOption)->pos = Vec3f(w / 2.0f, h / 2.0f - 50.0f, 0.0f);
	scene.Assign<CPostProcessing>(quitOption);

	EntityID buttonSelector =scene.NewEntity("Button Selector");
	CDrawable* pDrawable = scene.Assign<CDrawable>(buttonSelector);
	pDrawable->vertices = {
			Vertex(Vec3f(0.f, 0.0f, 0.f)),
			Vertex(Vec3f(0.7f, 0.5f, 0.f)),
			Vertex(Vec3f(0.f, 1.f, 0.f)),
		};
	pDrawable->indices = { 2, 0, 1, 2, 0, 1 };
	scene.Assign<CVisibility>(buttonSelector);
	CTransform* pTransform = scene.Assign<CTransform>(buttonSelector);
	pTransform->pos = Vec3f(w / 2.0f - 100.0f, h / 2.0f + 18.0f, 0.0f);
	pTransform->sca = Vec3f(30.f, 30.0f, 1.0f);

	return &scene;
}

int main(int argc, char *argv[])
{
	Engine::Initialize();

	// Type system testing
	{
		Component testComponent;

		TypeData& typeData = TypeDatabase::Get<Component>();

		TypeData& sameTypeData = TypeDatabase::GetFromString("Component");
		TypeData& intTypeData = TypeDatabase::GetFromString("int");

		Member& myInMember = typeData.GetMember("myInt");

		bool isInt = myInMember.IsType<int>();

		myInMember.Set(&testComponent, 1337);

		Log::Print(Log::EMsg, "Iterator printing Members of type: %s", typeData.name);
		for (Member& member : typeData)
		{			
			Log::Print(Log::EMsg, "Name: %s Type: %s val: %i", member.name, member.GetType().name, *member.Get<int>(&testComponent));
		}
	}

	// Make the main scene
	pMainScene = CreateMainAsteroidsScene();
	pMainMenuScene = CreateMainMenuScene();

	// Run everything
	Engine::Run(pMainScene);

	return 0;
}