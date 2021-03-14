#include "Asteroids.h"
#include "Systems_Old.h"
#include "Components.h"
#include "MovementSystem.h"
#include "PolylineDrawSystem.h"
#include "PlayerController.h"
#include "PlayerDeathSystem.h"
#include "CollisionSystem.h"

#include <Vec4.h>
#include <Matrix.h>
#include <Editor/Editor.h>
#include <Scene.h>
#include <Engine.h>
#include <Input/Input.h>
#include <Profiler.h>
#include <Rendering/FontSystem.h>
#include <Rendering/ParticlesSystem.h>
#include <Rendering/GameRenderer.h>
#include <TypeSystem.h>
#include <AudioDevice.h>

#include <Variant.h>

#include <World.h>
#include <Systems.h>
#include <Rendering/SceneDrawSystem.h>

#include <SDL.h>

eastl::fixed_vector<Vec2f, 15> GetRandomAsteroidMesh()
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

	eastl::fixed_vector<Vec2f, 15> vec;
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

World* CreateMainAsteroidsScene()
{
	World& world = *(new World());

	const float w = GameRenderer::GetWidth();
	const float h = GameRenderer::GetHeight();

	srand(unsigned int(time(nullptr)));
	auto randf = []() { return float(rand()) / float(RAND_MAX); };

	// Create the ship
	Vec2f playerVerts[5] = {
		Vec2f(0.f, 0.5f),
		Vec2f(1.f, 0.8f),
		Vec2f(0.9f, 0.7f),
		Vec2f(0.9f, 0.3f),
		Vec2f(1.0f, 0.2f)
	};
	Entity* pPlayerEnt = world.NewEntity("Player Ship");

	pPlayerEnt->AddNewSystem<PlayerController>();
	pPlayerEnt->AddNewSystem<PlayerDeathSystem>();

	PlayerComponent* pPlayer = pPlayerEnt->AddNewComponent<PlayerComponent>();

	SpatialComponent* pRoot = pPlayerEnt->AddNewComponent<SpatialComponent>();

	AsteroidPhysics* pRootPhysics = pPlayerEnt->AddNewComponent<AsteroidPhysics>(pRoot->GetId());
	pRootPhysics->SetLocalPosition(Vec3f(w / 2.0f, h / 2.0f, 0.0f));
	pRootPhysics->SetLocalScale(Vec3f(30.f, 35.f, 1.0f));
	pRootPhysics->type = CollisionType::Player;
	pRootPhysics->collisionRadius = 17.0f;

	Polyline* pPlayerPolyline = pPlayerEnt->AddNewComponent<Polyline>(pRootPhysics->GetId());
	pPlayerPolyline->points.assign(playerVerts, playerVerts + 5);

	pPlayer->playerPolylineComponent = pPlayerPolyline->GetId();

	// Create the lives
	float offset = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		Polyline* pLifePolyline = pPlayerEnt->AddNewComponent<Polyline>(pRoot->GetId());
		pLifePolyline->points.assign(playerVerts, playerVerts + 5);
		pLifePolyline->SetLocalPosition(Vec3f(150.f + offset, h - 85.0f, 0.0f));
		pLifePolyline->SetLocalScale(Vec3f(30.f, 35.f, 1.0f));
		pLifePolyline->SetLocalRotation(Vec3f(0.0f, 0.0f, -3.14159f / 2.0f));
		offset += 30.0f;

		pPlayer->lives.push_back(pLifePolyline->GetId());
	}

	// Create some asteroids
	for (int i = 0; i < 10; i++)
	{
		Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
		Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
		float randomRotation = randf() * 6.282f;

		Entity* pAsteroid = world.NewEntity("Asteroid");
		AsteroidComponent* pTemp = pAsteroid->AddNewComponent<AsteroidComponent>();

		AsteroidPhysics* pPhysics = pAsteroid->AddNewComponent<AsteroidPhysics>();
		pPhysics->velocity = randomVelocity;
		pPhysics->SetLocalPosition(randomLocation);
		pPhysics->SetLocalScale(Vec3f(90.0f, 90.0f, 1.0f));
		pPhysics->SetLocalRotation(Vec3f(0.0f, 0.0f, randomRotation));
		pPhysics->collisionRadius = 40.0f;

		Polyline* pAsteroidPoly = pAsteroid->AddNewComponent<Polyline>(pPhysics->GetId());
		pAsteroidPoly->points = GetRandomAsteroidMesh();
	}

	// Create the UI entity
	{
		Entity* pUIEntity = world.NewEntity("UI");

		SpatialComponent* pUIRoot = pUIEntity->AddNewComponent<SpatialComponent>();

		TextComponent* pScore = pUIEntity->AddNewComponent<TextComponent>(pUIRoot->GetId());
		pScore->SetLocalPosition(Vec3f(150.0f, h - 53.0f, 0.0f));
		pScore->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
		pScore->text = "0";

		TextComponent* pHighScore = pUIEntity->AddNewComponent<TextComponent>(pUIRoot->GetId());
		pHighScore->SetLocalPosition(Vec3f(w - 150.f, h - 53.0f, 0.0f));
		pHighScore->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
		pHighScore->text = "0";
	}

	world.AddGlobalSystem<PolylineDrawSystem>();
	world.AddGlobalSystem<MovementSystem>();
	world.AddGlobalSystem<FontDrawSystem>();
	world.AddGlobalSystem<CollisionSystem>();

	return &world;
}

World* CreateMainMenuScene()
{
	World& world = *(new World());

	const float w = GameRenderer::GetWidth();
	const float h = GameRenderer::GetHeight();

	Entity* pTitle = world.NewEntity("Main Menu Title");
	TextComponent* pTitleText = pTitle->AddNewComponent<TextComponent>();
	pTitleText->SetLocalPosition(Vec3f(w / 2.0f, h / 2.0f + 200.0f, 0.0f));
	pTitleText->SetLocalScale(Vec3f(2.0f, 2.0f, 2.0f));
	pTitleText->text = "Asteroids!";
	pTitleText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

	Entity* pStart = world.NewEntity("Start Option");
	TextComponent* pStartText = pStart->AddNewComponent<TextComponent>();
	pStartText->SetLocalPosition(Vec3f(w / 2.0f, h / 2.0f, 0.0f));
	pStartText->text = "Start";
	pStartText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
	
	Entity* pQuit = world.NewEntity("Quit Option");
	TextComponent* pQuitText = pQuit->AddNewComponent<TextComponent>();
	pQuitText->SetLocalPosition(Vec3f(w / 2.0f, h / 2.0f - 80.0f, 0.0f));
	pQuitText->text = "Quit";
	pQuitText->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");

	Entity* pSelector = world.NewEntity("Button Selector");
	Vec2f verts[3] = {
		Vec2f(0.f, 0.0f),
		Vec2f(0.7f, 0.5f),
		Vec2f(0.f, 1.f)
	};
	Polyline* pPolyline = pSelector->AddNewComponent<Polyline>();
	pPolyline->points.assign(verts, verts + 3);
	pPolyline->SetLocalPosition(Vec3f(w / 2.0f - 100.0f, h / 2.0f + 18.0f, 0.0f));
	pPolyline->SetLocalScale(Vec3f(30.f, 30.0f, 1.0f));

	world.AddGlobalSystem<PolylineDrawSystem>();
	world.AddGlobalSystem<FontDrawSystem>();

	return &world;
}

void LoadMainScene()
{
	Engine::SetActiveWorld(CreateMainAsteroidsScene());
}

void LoadMenu()
{
	Engine::SetActiveWorld(CreateMainMenuScene());
}

int main(int argc, char *argv[])
{
	Engine::Initialize("Games/Asteroids/Asteroids.cfg");

	// Run everything
	Engine::Run(CreateMainAsteroidsScene());

	return 0;
}