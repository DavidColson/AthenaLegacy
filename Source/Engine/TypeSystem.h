#pragma once

#include <vector>
#include <unordered_map>
#include <map>

// Actual Type data

struct TypeData;

struct Member
{
  const char* m_name;
  size_t m_offset;
  TypeData* m_pType;

  template<typename T>
  bool IsType()
  {
    // #TODO: add actual proper equality check not this hack 
    return m_pType == &TypeDatabase::Get<T>(); 
  }

  TypeData& GetType()
  {
    return *m_pType;
  }

  template<typename T>
  T* Get(void* instance)
  {
    return reinterpret_cast<T*>((char*)instance + m_offset);
  }
  template<typename T>
  void Set(void* instance, T newValue)
  {
    *reinterpret_cast<T*>((char*)instance + m_offset) = newValue;
  }
};

struct TypeData
{
  const char* m_name;
  size_t m_size;
  std::map<std::string, Member> m_members;

  TypeData(void(*initFunc)(TypeData*)) : TypeData{ nullptr, 0 }
  {
	  initFunc(this);
  }
  TypeData(const char* name, size_t size) : m_name{name}, m_size{size} {}
  TypeData(const char* name, size_t size, const std::initializer_list<std::pair<const std::string, Member>>& init) : m_name{name}, m_size{size}, m_members{ init } {}

  Member& GetMember(const char* name);

   struct MemberIterator
  {
    MemberIterator(std::map<std::string, Member>::iterator it) : m_it(it) {}

    Member& operator*() const 
    { 
      return m_it->second;
    }

    bool operator==(const MemberIterator& other) const 
    {
      return m_it == other.m_it;
    }
    bool operator!=(const MemberIterator& other) const 
    {
      return m_it != other.m_it;
    }

    MemberIterator& operator++()
    {
      ++m_it;
      return *this;
    }

    std::map<std::string, Member>::iterator m_it;
  };

  const MemberIterator begin() 
  {
    return MemberIterator(m_members.begin());
  }

  const MemberIterator end()
  {
    return MemberIterator(m_members.end());
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
  // the decltype term here may result in invalid c++ if T doesn't contain m_typeData. 
  // As such if it doesn't, this template instantiation will be completely ignored by the compiler through SNIFAE. 
  // If it does contain m_typeData, this will be instantiated and we're all good.
  // See here for deep explanation: https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
  template<typename T, typename = int>
  struct IsReflected : std::false_type {};
  template<typename T>
  struct IsReflected<T, decltype((void) T::m_typeData, 0)> : std::true_type {};

  // We're switching template versions depending on whether T has been internally reflected (i.e. has an m_typeData member)
  template<typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
  static TypeData& Get()
  {
    return T::m_typeData;
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

    std::unordered_map<std::string, TypeData*> m_typeNames;
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
  static TypeData m_typeData;                \
  static void initReflection(TypeData* type);

#define REFLECT_BEGIN(Struct)\
  TypeData Struct::m_typeData{Struct::initReflection};\
  void Struct::initReflection(TypeData* typeData) {\
    using XX = Struct;\
    TypeDatabase::Data::Get().m_typeNames.emplace(#Struct, typeData);\
    typeData->m_name = #Struct;\
    typeData->m_size = sizeof(XX);\
    typeData->m_members = {

#define REFLECT_MEMBER(member)\
      {#member, {#member, offsetof(XX, member), &TypeDatabase::Get<decltype(XX::member)>()}},

#define REFLECT_END()\
    };\
  }


// Special version of the begin macro for types that are template specializations, such as Vec<float>
#define REFLECT_TEMPLATED_BEGIN(Struct)\
  TypeData Struct::m_typeData{Struct::initReflection};\
  template<>\
  void Struct::initReflection(TypeData* typeData) {\
    using XX = Struct;\
    TypeDatabase::Data::Get().m_typeNames.emplace(#Struct, typeData);\
    typeData->m_name = #Struct;\
    typeData->m_size = sizeof(XX);\
    typeData->m_members = {
