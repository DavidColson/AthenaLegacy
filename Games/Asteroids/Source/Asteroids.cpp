#include "Asteroids.h"
#include "Systems_Old.h"
#include "Components.h"
#include "AsteroidPhysicsSystem.h"
#include "PolylineDrawSystem.h"
#include "PlayerController.h"
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
	pPlayerEnt->AddNewComponent<PlayerComponent>();
	SpatialComponent* pRoot = pPlayerEnt->AddNewComponent<SpatialComponent>();
	AsteroidPhysics* pRootPhysics = pPlayerEnt->AddNewComponent<AsteroidPhysics>();
	pRootPhysics->SetParent(pRoot);
	pRootPhysics->SetLocalPosition(Vec3f(w / 2.0f, h / 2.0f, 0.0f));
	pRootPhysics->SetLocalScale(Vec3f(30.f, 35.f, 1.0f));
	pRootPhysics->type = CollisionType::Player;
	pRootPhysics->collisionRadius = 17.0f;

	Polyline* pPlayerPolyline = pPlayerEnt->AddNewComponent<Polyline>();
	pPlayerPolyline->SetParent(pRootPhysics);
	pPlayerPolyline->points.assign(playerVerts, playerVerts + 5);

	// Create the lives
	float offset = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		Polyline* pLifePolyline = pPlayerEnt->AddNewComponent<Polyline>();
		pLifePolyline->SetParent(pRoot);
		pLifePolyline->points.assign(playerVerts, playerVerts + 5);
		pLifePolyline->SetLocalPosition(Vec3f(150.f + offset, h - 85.0f, 0.0f));
		pLifePolyline->SetLocalScale(Vec3f(30.f, 35.f, 1.0f));
		pLifePolyline->SetLocalRotation(Vec3f(0.0f, 0.0f, -3.14159f / 2.0f));
		offset += 30.0f;
	}

	// Create some asteroids
	for (int i = 0; i < 10; i++)
	{
		Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
		Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
		float randomRotation = randf() * 6.282f;
		Entity* pAsteroid = world.NewEntity("Asteroid");
		AsteroidPhysics* pPhysics = pAsteroid->AddNewComponent<AsteroidPhysics>();
		pPhysics->velocity = randomVelocity;
		pPhysics->SetLocalPosition(randomLocation);
		pPhysics->SetLocalScale(Vec3f(90.0f, 90.0f, 1.0f));
		pPhysics->SetLocalRotation(Vec3f(0.0f, 0.0f, randomRotation));
		pPhysics->collisionRadius = 40.0f;

		Polyline* pAsteroidPoly = pAsteroid->AddNewComponent<Polyline>();
		pAsteroidPoly->SetParent(pPhysics);
		pAsteroidPoly->points = GetRandomAsteroidMesh();
	}

	// Create the UI entity
	{
		Entity* pUIEntity = world.NewEntity("UI");
		SpatialComponent* pUIRoot = pUIEntity->AddNewComponent<SpatialComponent>();
		TextComponent* pScore = pUIEntity->AddNewComponent<TextComponent>();
		pScore->SetParent(pUIRoot);
		pScore->SetLocalPosition(Vec3f(150.0f, h - 53.0f, 0.0f));
		pScore->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
		pScore->text = "0";

		TextComponent* pHighScore = pUIEntity->AddNewComponent<TextComponent>();
		pHighScore->SetParent(pUIRoot);
		pHighScore->SetLocalPosition(Vec3f(w - 150.f, h - 53.0f, 0.0f));
		pHighScore->fontAsset = AssetHandle("Fonts/Hyperspace/Hyperspace Bold.otf");
		pHighScore->text = "0";
	}

	world.AddGlobalSystem<PolylineDrawSystem>();
	world.AddGlobalSystem<AsteroidPhysicsSystem>();
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