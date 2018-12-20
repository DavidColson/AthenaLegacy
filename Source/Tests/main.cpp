#define CATCH_CONFIG_RUNNER
#include "Catch/catch.hpp"
#include "SDL.h"

#include <Maths/Maths.h>

TEST_CASE("vec2 addition", "[vec2]")
{
	vec2 v = vec2(2.f, 1.0f);
	vec2 v2 = vec2(4.f, 5.0f);

	REQUIRE(v + v2 == vec2(6.0f, 6.0f));
	REQUIRE(v - v2 == vec2(-2.0f, -4.0f));
}

int main(int argc, char *argv[])
{
	int result = Catch::Session().run(argc, argv);
	return 0;
}