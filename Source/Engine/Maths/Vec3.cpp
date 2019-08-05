#include "Vec3.h"
#include "TypeData.h"

REGISTER(Vec3f)
{
	NewType(Vec3f)
		->RegisterMember("x", &Vec3f::x)
		->RegisterMember("y", &Vec3f::y)
		->RegisterMember("z", &Vec3f::z);
}

REGISTER(Vec3d)
{
	NewType(Vec3d)
		->RegisterMember("x", &Vec3d::x)
		->RegisterMember("y", &Vec3d::y)
		->RegisterMember("z", &Vec3d::z);
}