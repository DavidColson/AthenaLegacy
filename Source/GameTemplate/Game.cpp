#include "Game.h"
#include "Systems.h"
#include "Components.h"

#include <SDL.h>

int main(int argc, char *argv[])
{
	Engine::Initialize();

	// Run everything
	Engine::Run(new Scene());

	return 0;
}