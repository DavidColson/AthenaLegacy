#include <string>

#include "TypeSystem.h"
#include "ErrorHandling.h"

Member& TypeData::GetMember(const char* name)
{
  ASSERT(m_members.count(name) == 1, "The member you're trying to access doesn't exist");
  return m_members[name];
}

namespace TypeDatabase
{
  Data* TypeDatabase::Data::pInstance{ nullptr };

  TypeData& GetFromString(const char* name)
  {
    return *Data::Get().m_typeNames[name];
  }
}

// Primitive Types
//////////////////

struct TypeData_Int : TypeData
{
  TypeData_Int() : TypeData{"int", sizeof(int)} 
  {
    TypeDatabase::Data::Get().m_typeNames.emplace("int", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<int>()
{
  static TypeData_Int typeData;
  return typeData;
}

struct TypeData_Float : TypeData
{
  TypeData_Float() : TypeData{"float", sizeof(float)} 
  {
    TypeDatabase::Data::Get().m_typeNames.emplace("float", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<float>()
{
  static TypeData_Float typeData;
  return typeData;
}

struct TypeData_Double : TypeData
{
  TypeData_Double() : TypeData{"double", sizeof(double)} 
  {
    TypeDatabase::Data::Get().m_typeNames.emplace("double", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<double>()
{
  static TypeData_Double typeData;
  return typeData;
}

struct TypeData_String : TypeData
{
  TypeData_String() : TypeData{"std::string", sizeof(std::string)} 
  {
    TypeDatabase::Data::Get().m_typeNames.emplace("std::string", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<std::string>()
{
  static TypeData_String typeData;
  return typeData;
}

struct TypeData_Bool : TypeData
{
  TypeData_Bool() : TypeData{"bool", sizeof(bool)} 
  {
    TypeDatabase::Data::Get().m_typeNames.emplace("bool", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<bool>()
{
  static TypeData_Bool typeData;
  return typeData;
}