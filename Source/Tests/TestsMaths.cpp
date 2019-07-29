#include "Maths/Vec2.h"
#include "Catch/catch.hpp"

TEST_CASE("Vec2")
{
	// Creating a vector
	// *****************

	Vec2f unsetVec; // unset vectors will initialize automatically to the typename's default value 
	Vec2f oneVec(1.0f); // can set a vector with a single value
	Vec2f sampleVec(5.0f, 2.0f);
	Vec2f secondSampleVec(2.0f, 7.0f);

	Vec2d unsetDoubleVec;
	Vec2d oneDoubleVec(1.0);
	Vec2d sampleDoubleVec(12.0, 6.0);

	SECTION("Maths Operations") 
	{
		REQUIRE(sampleVec * 5.0f == Vec2f(25.f, 10.0f));

		REQUIRE(sampleVec / 2.0f == Vec2f(2.5f, 1.0f));

		REQUIRE(sampleVec + 3.0f == Vec2f(8.0f, 5.0f));

		REQUIRE(sampleVec - 3.0f == Vec2f(2.0f, -1.0f));

		REQUIRE(sampleVec + oneVec == Vec2f(6.0f, 3.0f));

		REQUIRE(sampleVec - oneVec == Vec2f(4.0f, 1.0f));

		REQUIRE(Vec2f::CompMul(sampleVec, secondSampleVec) == Vec2f(10.f, 14.0f));

		REQUIRE(Vec2f::CompDiv(sampleVec, secondSampleVec) == Vec2f(2.5f, 2.0f / 7.0f));

		REQUIRE(Vec2f::Dot(sampleVec, secondSampleVec) == 24.0f);

		REQUIRE(Vec2f::Cross(sampleVec, secondSampleVec) == -4.0f);

		REQUIRE(sampleVec.GetLength() == sqrt(29.0f));

		REQUIRE(sampleVec.GetNormalized().GetLength() == 1.0f);

		REQUIRE(-sampleVec == Vec2f(-5.0f, -2.0f));
	}

	SECTION("Assignment And Equality Operations")
	{
		REQUIRE(sampleVec != secondSampleVec);

		Vec2f temp(0.0f);
		temp += 5.0f;
		REQUIRE(temp == Vec2f(5.0f, 5.0f));

		temp -= 4.0f;
		REQUIRE(temp == Vec2f(1.0f));

		temp *= 2.0f;
		REQUIRE(temp == Vec2f(2.0f, 2.0f));

		temp /= 2.0f;
		REQUIRE(temp == Vec2f(1.0f, 1.0f));

		temp += Vec2f(1.0f, 2.0f);
		REQUIRE(temp == Vec2f(2.0f, 3.0f));

		temp -= Vec2f(2.0f, 1.0f);
		REQUIRE(temp == Vec2f(0.0f, 2.0f));
	}

	SECTION("Component Access")
	{
		REQUIRE(sampleVec[0] == 5.0f);

		REQUIRE(sampleVec[1] == 2.0f);

		Vec2f temp = sampleVec;
		temp[0] = 17.0f;
		REQUIRE(temp == Vec2f(17.0f, 2.0f));
	}
}