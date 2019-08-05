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