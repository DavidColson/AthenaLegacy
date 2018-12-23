#include "TypeData.h"
#define CATCH_CONFIG_RUNNER
#include "Catch/catch.hpp"
#include "SDL.h"

int main(int argc, char *argv[])
{
	int result = Catch::Session().run(argc, argv);
	return 0;
}