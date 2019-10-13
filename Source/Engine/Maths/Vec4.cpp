#include "Vec4.h"
#include "TypeData.h"

REGISTER(Vec4f)
{
	NewType(Vec4f)
		->RegisterMember("x", &Vec4f::x)
		->RegisterMember("y", &Vec4f::y)
		->RegisterMember("z", &Vec4f::z)
		->RegisterMember("w", &Vec4f::w);
}

REGISTER(Vec4d)
{
	NewType(Vec4d)
		->RegisterMember("x", &Vec4d::x)
		->RegisterMember("y", &Vec4d::y)
		->RegisterMember("z", &Vec4d::z)
		->RegisterMember("w", &Vec4d::w);
}

REFLECT_TEMPLATED_BEGIN(Vec4f)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_MEMBER(z)
REFLECT_MEMBER(w)
REFLECT_END()

REFLECT_TEMPLATED_BEGIN(Vec4d)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_MEMBER(z)
REFLECT_MEMBER(w)
REFLECT_END()