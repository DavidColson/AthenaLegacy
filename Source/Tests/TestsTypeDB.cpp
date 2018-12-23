#include "TypeDB.h"
#include "Catch/catch.hpp"


TEST_CASE("TypeDB Variants", "[TypeDB]")
{
	using namespace TypeDB;

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

TEST_CASE("TypeDB RefVariants", "[TypeDB]")
{
	using namespace TypeDB;

	// RefVariants are a bit like variants, but they don't copy values in, just store references
	RefVariant variantFloat = 5.0f;
	RefVariant variantString = std::string("chicken");

	REQUIRE(variantFloat.IsA<float>());
	REQUIRE(variantString.IsA<std::string>());

	// Notice that the variant updates when we change testFloat
	float testFloat = 5.0f;
	variantFloat = testFloat;
	REQUIRE(variantFloat.Get<float>() == 5.0f);
	testFloat = 7.0f;
	REQUIRE(variantFloat.Get<float>() == 7.0f);

	// Ref variants can be constructed from normal variants
	Variant variant = 1337;
	RefVariant refVar = variant;
	REQUIRE(refVar.Get<int>() == 1337);
	variant = 27;
	REQUIRE(refVar.Get<int>() == 27);

	// Note that changing the referenced variant's type will lead to undefined behaviour
	variant = std::string("ducks");
	REQUIRE(!refVar.IsA<std::string>());
}

TEST_CASE("TypeDB Types", "[TypeDB]")
{
	using namespace TypeDB;

	// Registering a type
	struct PODType
	{
		float number = 5.0f;
	};
	NewType(PODType)->RegisterMember("number", &PODType::number);

	// We can read information from the type like the size
	REQUIRE(GetType<PODType>()->m_size == sizeof(PODType));

	// Get type info from a string and check equality
	REQUIRE(GetTypeFromString("PODType") == GetType<PODType>());

	// Get type info from an instance
	PODType realInstance;
	REQUIRE(GetType(realInstance) == GetType<PODType>());

	// Construct new instances of a type
	Variant instance = GetType<PODType>()->New();
	REQUIRE(instance.IsA<PODType>());
}

TEST_CASE("TypeDB Members", "[TypeDB]")
{
	using namespace TypeDB;

	// Registering a type
	struct PODType
	{
		float number = 5.0f;
	};
	NewType(PODType)->RegisterMember("number", &PODType::number);

	Variant instance = GetType<PODType>()->New();

	// We can manipulate members of the type
	Member* numMember = GetType<PODType>()->GetMember("number");
	REQUIRE(numMember->IsType<float>());
	REQUIRE(numMember->IsType("float"));

	// Get the value of the member
	REQUIRE(numMember->GetValue<float>(instance) == 5.0f);

	// Set the value of a member
	numMember->SetValue(instance, 2.0f);
	REQUIRE(numMember->GetValue<float>(instance) == 2.0f);

	// We can also modify members of typed instances
	PODType typedInstance;
	numMember->SetValue(typedInstance, 7.0f);
	REQUIRE(typedInstance.number == 7.0f);

	// We can also modify members by using GetRefValue
	numMember->GetRefValue<float>(typedInstance) = 20.f;
	REQUIRE(typedInstance.number == 20.f);
}

