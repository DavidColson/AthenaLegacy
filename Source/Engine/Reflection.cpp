#include "Reflection.h"

std::unordered_map<TypeId, TypeDatabase::Type> TypeDatabase::Detail::typeDatabase;

REGISTRATION
{
	RegisterNewType(std::string);
	RegisterNewType(float);
	RegisterNewType(int);
}