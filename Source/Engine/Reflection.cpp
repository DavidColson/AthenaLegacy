#include "Reflection.h"

std::unordered_map<std::string, TypeData> m_typeDatabase;

TypeData* TypeDatabase::GetTypeData(std::string typeName)
{
	return &m_typeDatabase[typeName];
}

TypeData* TypeDatabase::RegisterNewType(std::string typeName)
{
	m_typeDatabase.emplace(typeName, TypeData());
	return &m_typeDatabase[typeName];
}
