#include "Reflection.h"

std::unordered_map<TypeId, TypeDatabase::Type> TypeDatabase::Detail::typeDatabase;

REGISTRATION
{
	TypeDatabase::RegisterNewType<float>("float");
	TypeDatabase::RegisterNewType<int>("int");
}