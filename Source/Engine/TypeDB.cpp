#include "TypeDB.h"
#include "TypeData.h"

using namespace TypeDB;

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


// Member
/////////

Type* TypeDB::Member::GetType()
{
	return m_type;
}

bool TypeDB::Member::IsType(std::string typeName)
{
	return TypeDB::GetTypeFromString(typeName) == m_type;
}


// Type
///////


bool Type::operator==(const Type* other)
{
	return this->m_id == other->m_id;
}

Member* Type::GetMember(const char* name)
{
	ASSERT(m_memberList.count(name) == 1, "The member you're trying to access doesn't exist");
	return m_memberList[name];
}

Variant Type::New()
{
	return m_constructor->Invoke();
}

Type* TypeDB::GetTypeFromString(std::string typeName)
{
	ASSERT(Detail::Data::Get().typeNames.count(typeName) == 1, "The type you are querying does not exist in the database, please register it");
	return &Detail::Data::Get().typeDatabase[Detail::Data::Get().typeNames[typeName]];
}

Type* TypeDB::GetType(TypeId typeId)
{
	ASSERT(Detail::Data::Get().typeDatabase.count(typeId) == 1, "The type you are querying does not exist in the database, please register it");
	return &Detail::Data::Get().typeDatabase[typeId];
}

REGISTER(float)
{
	NewType(std::string);
	NewType(float);
	NewType(float*);
	NewType(int);
	NewType(int*);
	NewType(bool);
}

TypeDB::Detail::Data* TypeDB::Detail::Data::pInstance{ nullptr };
