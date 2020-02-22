#include <string>

#include "TypeSystem.h"
#include "ErrorHandling.h"

Member& TypeData::GetMember(const char* _name)
{
  ASSERT(members.count(_name) == 1, "The member you're trying to access doesn't exist");
  return members[_name];
}

namespace TypeDatabase
{
  Data* TypeDatabase::Data::pInstance{ nullptr };

  TypeData& GetFromString(const char* name)
  {
    return *Data::Get().typeNames[name];
  }
}

// Primitive Types
//////////////////

struct TypeData_Int : TypeData
{
  TypeData_Int() : TypeData{"int", sizeof(int)} 
  {
    TypeDatabase::Data::Get().typeNames.emplace("int", this);
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
    TypeDatabase::Data::Get().typeNames.emplace("float", this);
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
    TypeDatabase::Data::Get().typeNames.emplace("double", this);
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
    TypeDatabase::Data::Get().typeNames.emplace("std::string", this);
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
    TypeDatabase::Data::Get().typeNames.emplace("bool", this);
  }
};
template <>
TypeData& getPrimitiveTypeData<bool>()
{
  static TypeData_Bool typeData;
  return typeData;
}