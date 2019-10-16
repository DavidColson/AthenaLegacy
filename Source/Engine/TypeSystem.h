#pragma once

#include <vector>
#include <unordered_map>
#include <map>

// Actual Type data

struct TypeData;

struct Member
{
  const char* name;
  size_t offset;
  TypeData* pType;

  template<typename T>
  bool IsType()
  {
    // #TODO: add actual proper equality check not this hack 
    return pType == &TypeDatabase::Get<T>(); 
  }

  TypeData& GetType()
  {
    return *pType;
  }

  template<typename T>
  T* Get(void* instance)
  {
    return reinterpret_cast<T*>((char*)instance + offset);
  }
  template<typename T>
  void Set(void* instance, T newValue)
  {
    *reinterpret_cast<T*>((char*)instance + offset) = newValue;
  }
};

struct TypeData
{
  const char* name;
  size_t size;
  std::map<std::string, Member> members;

  TypeData(void(*initFunc)(TypeData*)) : TypeData{ nullptr, 0 }
  {
	  initFunc(this);
  }
  TypeData(const char* _name, size_t _size) : name(_name), size(_size) {}
  TypeData(const char* _name, size_t _size, const std::initializer_list<std::pair<const std::string, Member>>& init) : name(_name), size(_size), members( init ) {}

  Member& GetMember(const char* _name);

   struct MemberIterator
  {
    MemberIterator(std::map<std::string, Member>::iterator _it) : it(_it) {}

    Member& operator*() const 
    { 
      return it->second;
    }

    bool operator==(const MemberIterator& other) const 
    {
      return it == other.it;
    }
    bool operator!=(const MemberIterator& other) const 
    {
      return it != other.it;
    }

    MemberIterator& operator++()
    {
      ++it;
      return *this;
    }

    std::map<std::string, Member>::iterator it;
  };

  const MemberIterator begin() 
  {
    return MemberIterator(members.begin());
  }

  const MemberIterator end()
  {
    return MemberIterator(members.end());
  }
};

// Type resolving mechanism
template<typename T>
TypeData& getPrimitiveTypeData() 
{
  static TypeData unknownType("UnkownType", 0); 
  return unknownType;
}

template <>
TypeData& getPrimitiveTypeData<int>();
template <>
TypeData& getPrimitiveTypeData<float>();
template <>
TypeData& getPrimitiveTypeData<double>();
template <>
TypeData& getPrimitiveTypeData<std::string>();
template <>
TypeData& getPrimitiveTypeData<bool>();

struct DefaultTypeResolver 
{
  // the decltype term here may result in invalid c++ if T doesn't contain typeData. 
  // As such if it doesn't, this template instantiation will be completely ignored by the compiler through SFINAE. 
  // If it does contain typeData, this will be instantiated and we're all good.
  // See here for deep explanation: https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
  template<typename T, typename = int>
  struct IsReflected : std::false_type {};
  template<typename T>
  struct IsReflected<T, decltype((void) T::typeData, 0)> : std::true_type {};

  // We're switching template versions depending on whether T has been internally reflected (i.e. has an typeData member)
  template<typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
  static TypeData& Get()
  {
    return T::typeData;
  }
  template<typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
  static TypeData& Get()
  {
    return getPrimitiveTypeData<T>();
  }
};


namespace TypeDatabase
{
  // For storing lookup tables and other static type information
  struct Data
  {
    static Data& Get() 
    {
      if (pInstance == nullptr)
        pInstance = new Data();
      return *pInstance;
    }
    static Data* pInstance;

    std::unordered_map<std::string, TypeData*> typeNames;
  };

  TypeData& GetFromString(const char* name);

  // Generic typedata return
  template<typename T>
  TypeData& Get()
  {
    return DefaultTypeResolver::Get<T>();
  }
};




// Reflection macros
#define REFLECT()                               \
  static TypeData typeData;                \
  static void initReflection(TypeData* type);

#define REFLECT_BEGIN(Struct)\
  TypeData Struct::typeData{Struct::initReflection};\
  void Struct::initReflection(TypeData* typeData) {\
    using XX = Struct;\
    TypeDatabase::Data::Get().typeNames.emplace(#Struct, typeData);\
    typeData->name = #Struct;\
    typeData->size = sizeof(XX);\
    typeData->members = {

#define REFLECT_MEMBER(member)\
      {#member, {#member, offsetof(XX, member), &TypeDatabase::Get<decltype(XX::member)>()}},

#define REFLECT_END()\
    };\
  }


// Special version of the begin macro for types that are template specializations, such as Vec<float>
#define REFLECT_TEMPLATED_BEGIN(Struct)\
  TypeData Struct::typeData{Struct::initReflection};\
  template<>\
  void Struct::initReflection(TypeData* typeData) {\
    using XX = Struct;\
    TypeDatabase::Data::Get().typeNames.emplace(#Struct, typeData);\
    typeData->name = #Struct;\
    typeData->size = sizeof(XX);\
    typeData->members = {
