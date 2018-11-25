#include "Reflection.h"

using namespace TypeDB;

std::unordered_map<TypeId, Type> Detail::typeDatabase;
std::unordered_map<std::string, TypeId> Detail::typeNames;



// Variant
//////////

Variant::~Variant()
{
	delete[] reinterpret_cast<char *>(m_data);
}

Variant::Variant(const Variant& copy)
{
	m_type = copy.m_type;
	memcpy(m_data, copy.m_data, m_type->m_size);
}



// RefVariant
/////////////


RefVariant::RefVariant(const VariantBase& copy)
{
	m_type = copy.m_type;
	m_data = copy.m_data;
}

RefVariant::RefVariant(const Variant& copy)
{
	m_type = copy.m_type;
	m_data = copy.m_data;
}

RefVariant::RefVariant(const RefVariant& copy)
{
	m_type = copy.m_type;
	m_data = copy.m_data;
}


// Type
///////


bool Type::operator==(const Type* other)
{
	return this->m_id == other->m_id;
}

Member* Type::GetMember(const char* name)
{
	assert(m_memberList.count(name) == 1); // The member you're trying to access doesn't exist
	return m_memberList[name];
}

Variant Type::New()
{
	return m_constructor->Invoke();
}


Type* TypeDB::GetTypeFromString(std::string typeName)
{
	assert(Detail::typeNames.count(typeName) == 1); // The type you are querying does not exist in the database, please register it
	return &Detail::typeDatabase[Detail::typeNames[typeName]];
}

Type* TypeDB::GetType(TypeId typeId)
{
	assert(Detail::typeDatabase.count(typeId) == 1); // The type you are querying does not exist in the database, please register it
	return &Detail::typeDatabase[typeId];
}

REGISTRATION
{
	RegisterNewType(std::string);
	RegisterNewType(float);
	RegisterNewType(float*);
	RegisterNewType(int);
	RegisterNewType(int*);
	RegisterNewType(bool);
}