#include "Asteroids.h"
#include "Systems_Old.h"
#include "Components.h"
#include "PolylineDrawSystem.h"

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

#include <World.h>
#include <Systems.h>
#include <Rendering/SceneDrawSystem.h>

#include <SDL.h>

World* CreateMainAsteroidsScene()
{
	World& world = *(new World());

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
	Engine::Run(CreateMainMenuScene());

	return 0;
}