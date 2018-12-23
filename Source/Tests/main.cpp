#include "TypeData.h"
#define CATCH_CONFIG_RUNNER
#include "Catch/catch.hpp"
#include "SDL.h"

#include <Maths/Maths.h>

TEST_CASE("vec2", "[vec2]")
{
	vec2 v = vec2(2.f, 1.0f);
	vec2 v2 = vec2(4.f, 5.0f);

	SECTION("addition and subtraction")
	{
		REQUIRE(v + v2 == vec2(6.0f, 6.0f));
		REQUIRE(v - v2 == vec2(-2.0f, -4.0f));
	}
}

// Entity system
/**
 * Entity creation and deletion
 * Getting components before and after deletion
 * Create, delete re-create entity, check it's got the same index from the free list
 * check for exception when making too many entities

 * Component assignment
 * Check component properties are valid after assignment
 * Component getting
 * Component getting on an entity without that component
 * Check has component before and after component assignment
 * HasComponent on deleted entities
 * pass invalid entity ids to *component functions
 * 
 * Sceneview looping over correct number of entities
 * sceneview behaviour when deleting and recreating entities in the loop
 * sceneview behaviour when entities are at end and beginning of list
 * Sceneview iterating over subsets of entities within the list
 */

int main(int argc, char *argv[])
{
	int result = Catch::Session().run(argc, argv);
	return 0;
}