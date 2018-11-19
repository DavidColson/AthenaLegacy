#include "Reflection.h"

std::unordered_map<TypeId, TypeDatabase::Type> TypeDatabase::Detail::typeDatabase;

REGISTRATION
{
	RegisterNewType(float);
	RegisterNewType(int);
}