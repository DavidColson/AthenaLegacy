#include "Pigeons.h"
#include "Systems.h"
#include "Components.h"
#include "Json.h"
#include "SceneSerializer.h"
#include "FileSystem.h"
#include "World.h"

#include <SDL.h>

int main(int argc, char *argv[])
{
	Engine::Initialize("Games/Pigeons/Pigeons.cfg");

	JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile("Games/PigeonGame/Resources/Levels/PigeonScene.lvl"));
	Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);

	World world;

	// Run everything
	Engine::Run(pScene, &world);

	return 0;
}