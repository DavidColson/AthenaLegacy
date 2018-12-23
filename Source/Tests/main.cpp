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


// Reflection system
/**
 * Tests have their own register types
 *
 * RefVariants:
 * creating ref variants, modify underlying data and check change is recorded
 * Check copy functions from different variant types
 * Make copies of ref variants, and change underlying data
 * make ref variants from real variants, changing data and checking it's there
 * assignment to refvariants
 *
 * Type:
 * Check equality checks
 * Check new constructor
 * Check get type for instances, typeIds, strings, and literal types
 * 
 * Member:
 * Use setValue and compare underlying data
 * Use refvalue and compare data
 * Test cases for the uses in editor code
 * Check for assertions when providing the wrong type to member get/set
 *
 *
 * Aim for at least every function be tested, can add more tests later
 */

TEST_CASE("TypeDB Variants", "[TypeDB]")
{
	// Create variants of different literal types
	Variant variantFloat = 5.0f;
	Variant variantString = std::string("chicken");

	REQUIRE(variantFloat.IsA<float>());
	REQUIRE(variantString.IsA<std::string>());

	// Convert our variants back to their real types
	REQUIRE(variantFloat.Get<float>() == 5.0f);
	REQUIRE(variantString.Get<std::string>() == std::string("chicken"));

	// Create from copy
	Variant variantCopy = variantFloat;
	REQUIRE(variantCopy.IsA<float>());
	
	// Create variant, copy and change type
	Variant variantChanging = 1337;
	REQUIRE(variantChanging.IsA<int>());
	variantChanging = variantString;
	REQUIRE(variantChanging.IsA<std::string>());
	variantChanging = 25.0f;
	REQUIRE(variantChanging.IsA<float>());
	
	// Create from a typed variable
	float someFloat = 10.3f;
	Variant nonLiteralVar = someFloat;
	REQUIRE(nonLiteralVar.IsA<float>());

	// Variants copy their data, 
	// the exectation is that you cannot edit the values and expect the variant to update
	Variant varFloat = 7.0f;
	float theFloat = varFloat.Get<float>();
	theFloat = 2.0f;
	REQUIRE(varFloat.Get<float>() == 7.0f);
}

int main(int argc, char *argv[])
{
	int result = Catch::Session().run(argc, argv);
	return 0;
}