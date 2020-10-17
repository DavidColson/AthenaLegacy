#include "Pigeons.h"
#include "Systems.h"
#include "Components.h"
#include "Json.h"
#include "SceneSerializer.h"
#include "FileSystem.h"

#include <SDL.h>

int main(int argc, char *argv[])
{
	Engine::Initialize("Pigeons.cfg");

	JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile("Resources/Levels/PigeonScene.lvl"));
	Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);

	// Run everything
	Engine::Run(pScene);

	return 0;
}