#include "Pigeons.h"
#include "World.h"
#include "Entity.h"
#include "Rendering/SpriteDrawSystem.h"

#include <SDL.h>

int main(int argc, char *argv[])
{
	Engine::Initialize("Games/Pigeons/Pigeons.cfg");

	World* pWorld = new World();

	Entity* pEntity = pWorld->NewEntity("Pigeon");
	Sprite* pSprite = pEntity->AddNewComponent<Sprite>();
	pSprite->SetLocalPosition(Vec3f(612.0f, 378.0f, 0.0f));
	pSprite->SetLocalScale(Vec3f(200.0f, 200.0f, 0.0f));
	pSprite->spriteHandle = AssetHandle("Images/pigeon.png");

	pWorld->AddGlobalSystem<SpriteDrawSystem>();

	// Run everything
	Engine::Run(pWorld);

	return 0;
}