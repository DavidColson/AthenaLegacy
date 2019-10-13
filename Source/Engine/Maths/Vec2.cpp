#include "Vec2.h"
#include "TypeData.h"

REGISTER(Vec2f)
{
	NewType(Vec2f)
		->RegisterMember("x", &Vec2f::x)
		->RegisterMember("y", &Vec2f::y);
}

REGISTER(Vec2d)
{
	NewType(Vec2d)
		->RegisterMember("x", &Vec2d::x)
		->RegisterMember("y", &Vec2d::y);
}

REFLECT_TEMPLATED_BEGIN(Vec2f)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_END()

REFLECT_TEMPLATED_BEGIN(Vec2d)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_END()