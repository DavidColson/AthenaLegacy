#pragma once

#include <EASTL/string.h>
#include <EASTL/map.h>

#include "Variant.h"
#include "Json.h"
#include "StringHash.h"

struct TypeData;

struct Type {
    template<typename Type>
    inline static uint32_t Index()
	{
		static uint32_t typeIndex = TypeDatabase::Data::Get().typeCounter++;
		return typeIndex;
	}
};

struct Member
{
	const char* name;
	Member(const char* _name) : name(_name) {}

	template<typename T>
	bool IsType()
	{
		// This returns true if you ask if the item is an unknown type, which is real bad
		return GetType() == TypeDatabase::Get<T>(); 
	}

	virtual TypeData& GetType() = 0;
	virtual Variant Get(Variant& instance) = 0;
	virtual void Set(Variant& instance, Variant newValue) = 0;
};

template<typename MemberType, typename ClassType>
struct Member_Internal : public Member
{
	Member_Internal(const char* _name, MemberType ClassType::*pointer) : Member(_name), memberPointer(pointer) {}

	virtual TypeData& GetType() override
	{
		return TypeDatabase::Get<MemberType>();
	}

	virtual Variant Get(Variant& instance) override
	{
		return Variant(instance.GetValue<ClassType>().*memberPointer);
	}

	// Copy happening on value, consider replacing with argument wrapper
	virtual void Set(Variant& instance, Variant value) override
	{
		instance.GetValue<ClassType>().*memberPointer = value.GetValue<MemberType>();
	}

	MemberType ClassType::* memberPointer;
};


struct TypeDataOps
{
	virtual Variant New() = 0;
	virtual Variant CopyToVariant(void* pObject) = 0;
	virtual void PlacementNew(void* location) = 0;
	virtual void Destruct(void* pObject) = 0;
	virtual void Free(void* pObject) = 0;
};

// @Improvement Consider replacing this mechanism and component handler with the variant data policy method, 
// which is similiar, but a bit more robust and doesn't require "new" when creating these constructor objects
template<typename T>
struct TypeDataOps_Internal : public TypeDataOps
{
	virtual Variant New() override
	{
		return Variant(T());
	}

	virtual Variant CopyToVariant(void* pObject) override
	{
		return Variant(*reinterpret_cast<T*>(pObject));
	}

	virtual void PlacementNew(void* location) override
	{
		new (location) T();
	}

	virtual void Destruct(void* pObject) override
	{
		static_cast<T*>(pObject)->~T();
	}

	virtual void Free(void* pObject) override
	{
		T* pData = static_cast<T*>(pObject);
		delete pData;
	}
};

// Actual Type data
struct TypeData
{
	uint32_t id{ 0 };
	const char* name;
	size_t size;
	TypeDataOps* pConstructor{ nullptr };
	eastl::map<size_t, Member*> members;
	eastl::map<eastl::string, size_t> memberOffsets;

	// temp until we have type attributes
	bool isComponent{ false };

	TypeData(void(*initFunc)(TypeData*)) : TypeData{ nullptr, 0 }
	{
		initFunc(this);
	}
	TypeData(const char* _name, size_t _size) : name(_name), size(_size) {}
	TypeData(const char* _name, size_t _size, const std::initializer_list<eastl::map<size_t, Member*>::value_type>& init) : name(_name), size(_size), members( init ) {}

	~TypeData();
	Variant New();
	
	virtual JsonValue ToJson(Variant var);
	virtual Variant FromJson(const JsonValue& val);

	// Since type data is globally stored in the type database, equality checks can check the pointer addresses
	bool operator==(const TypeData& other)
	{
		return other.id == this->id;
	}
	bool operator!=(const TypeData& other)
	{
		return other.id != this->id;
	}

	bool IsValid()
	{
		return id != 0;
	}

	bool MemberExists(const char* _name);
	Member& GetMember(const char* _name);

	struct MemberIterator
	{
		MemberIterator(eastl::map<size_t, Member*>::iterator _it) : it(_it) {}

		Member& operator*() const 
		{ 
			return *it->second;
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

		eastl::map<size_t, Member*>::iterator it;
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
	static TypeData unknownType("UnknownType", 0); 
	return unknownType;
}

// @TODO: Move to BaseTypes.h/cpp that will include parsing for base types
template <>
TypeData& getPrimitiveTypeData<int>();
template <>
TypeData& getPrimitiveTypeData<float>();
template <>
TypeData& getPrimitiveTypeData<double>();
template <>
TypeData& getPrimitiveTypeData<eastl::string>();
template <>
TypeData& getPrimitiveTypeData<bool>();

struct EntityID;
template <>
TypeData& getPrimitiveTypeData<EntityID>();
struct AssetHandle;
template <>
TypeData& getPrimitiveTypeData<AssetHandle>();

struct DefaultTypeResolver 
{
	// the decltype term here may result in invalid c++ if T doesn't contain typeData. 
	// As such if it doesn't, this template instantiation will be completely ignored by the compiler through SFINAE. 
	// If it does contain typeData, this will be instantiated and we're all good.
	// See here for deep explanation: https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
	template<typename T, typename = int>
	struct IsReflected : eastl::false_type {};
	template<typename T>
	struct IsReflected<T, decltype((void) T::typeData, 0)> : eastl::true_type {};

	// We're switching template versions depending on whether T has been internally reflected (i.e. has an typeData member)
	template<typename T, eastl::enable_if_t<IsReflected<T>::value, int> = 0>
	static TypeData& Get()
	{
		return T::typeData;
	}
	template<typename T, eastl::enable_if_t<!IsReflected<T>::value, int> = 0>
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

		eastl::map<eastl::string, TypeData*> typeNames;
		uint32_t typeCounter{ 0 };
	};

	bool TypeExists(const char* name);
	TypeData& GetFromString(const char* name);

	template<typename T>
	bool TypeExists()
	{
		if (DefaultTypeResolver::Get<T>() == TypeData("UnknownType", 0))
			return false;
		return true;
	}

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
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pConstructor = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {

#define REFLECT_MEMBER(member)\
			{offsetof(XX, member), new Member_Internal<decltype(XX::member), XX>(#member, &XX::member)},

#define REFLECT_END()\
		};\
		for (const eastl::pair<size_t, Member*>& mem : selfTypeData->members) { selfTypeData->memberOffsets[mem.second->name] = mem.first; }\
	}


// Special version of the begin macro for types that are template specializations, such as Vec<float>
#define REFLECT_TEMPLATED_BEGIN(Struct)\
	TypeData Struct::typeData{Struct::initReflection};\
	template<>\
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pConstructor = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {

// Special version of reflection function used for components. Allows for storing specialized component handler object
#define REFLECT_COMPONENT_BEGIN(Struct)\
	TypeData Struct::typeData{Struct::initReflection};\
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->isComponent = true;\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pConstructor = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {