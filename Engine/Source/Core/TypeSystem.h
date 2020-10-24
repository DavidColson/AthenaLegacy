#pragma once

#include <EASTL/string.h>
#include <EASTL/map.h>

#include "Variant.h"
#include "Json.h"
#include "StringHash.h"

struct TypeDataOps;
struct Member;

struct TypeData
{
	/**
	 * Creates a new instance of this type, returning it as a variant
	 **/
	Variant New();
	
	/**
	 * Turns an instance of this type into a JsonValue structure for serialization to Json
	 **/
	virtual JsonValue ToJson(Variant var);

	/**
	 * Converts a Json structure into an instance of this type, returned as a variant
	 **/
	virtual Variant FromJson(const JsonValue& val);

	/**
	 * Checks for equality with another TypeData
	 **/
	bool operator==(const TypeData& other);

	/**
	 * Checks for inequality with another TypeData
	 **/
	bool operator!=(const TypeData& other);

	/**
	 * Returns false if this is an unknown, invalid typedata
	 **/
	bool IsValid();

	/**
	 * Checks for existence of a member by name
	 **/
	bool MemberExists(const char* _name);

	/**
	 * Retrieves reference to a member of this typeData
	 **/
	Member& GetMember(const char* _name);

	/**
	 * Defines a forward iterator on the members of this typedata
	 * 
	 * Use like: for(Member& mem : someTypeData) { ... }
	 * 
	 **/
	struct MemberIterator
	{
		MemberIterator(eastl::map<size_t, Member*>::iterator _it) : it(_it) {}

		Member& operator*() const;

		bool operator==(const MemberIterator& other) const;

		bool operator!=(const MemberIterator& other) const;

		MemberIterator& operator++();

		eastl::map<size_t, Member*>::iterator it;
	};

	/**
	 * Iterator to first member
	 **/
	const MemberIterator begin();

	/**
	 * Iterator to last member
	 **/
	const MemberIterator end();

	/**
	 * Constructors used for building typedatas, not for use outside of REFLECT macros
	 **/
	TypeData(void(*initFunc)(TypeData*)) : TypeData{ nullptr, 0 } { initFunc(this); }
	TypeData(const char* _name, size_t _size) : name(_name), size(_size) {}
	TypeData(const char* _name, size_t _size, const std::initializer_list<eastl::map<size_t, Member*>::value_type>& init) : name(_name), size(_size), members( init ) {}
	~TypeData();

	uint32_t id{ 0 };
	const char* name;
	size_t size;
	TypeDataOps* pTypeOps{ nullptr };
	eastl::map<size_t, Member*> members;
	eastl::map<eastl::string, size_t> memberOffsets;

	// temp until we have type attributes
	bool isComponent{ false };
};

struct Member
{
	/**
	 * String identifier of this member
	 **/
	const char* name;

	/**
	 * Check to determine if this member is of type T
	 **/
	template<typename T>
	bool IsType();

	/**
	 * Returns the typeData for the type of this member
	 **/
	virtual TypeData& GetType() = 0;

	/**
	 * Get an instance of a member from an instance of it's owning type
	 * Note that the return value will have copied the value into the variant
	 **/
	virtual Variant Get(Variant& instance) = 0;

	/**
	 * Set the member of instance to newValue
	 **/
	virtual void Set(Variant& instance, Variant newValue) = 0;
	
	/**
	 *	Constructor, for internal use in REFLEC_* macros, do not use elsewhere
	 **/
	Member(const char* _name) : name(_name) {}
};

namespace TypeDatabase
{
	/**
	 * Check for the existence of a type in the database by string name
	 **/
	bool TypeExists(const char* name);

	/**
	 * Retrieve a reference to a typeData from a string
	 **/
	TypeData& GetFromString(const char* name);

	/**
	 * Similar to above, checks for existing of Type T in the database
	 **/
	template<typename T>
	bool TypeExists();

	/**
	 *  Similiar to GetFromString, retrieves a reference to a TypeData of type T
	 **/
	template<typename T>
	TypeData& Get();
};

/**
 * Type Index retrieval
 * Note that these indexes are not stable between runs
 * Use like Type::Index<T>()
 **/
struct Type {
    template<typename Type>
    inline static uint32_t Index()
	{
		static uint32_t typeIndex = TypeDatabase::Data::Get().typeCounter++;
		return typeIndex;
	}
};

// *************************************
// REFLECTION MACROS
// **************************************

/**
 * Used to define a type as reflectable during type declaration 
 **/
#define REFLECT()                               \
	static TypeData typeData;                \
	static void initReflection(TypeData* type);

/**
 * Used to specify the structure of a type, use in cpp files
 **/
#define REFLECT_BEGIN(Struct)\
	TypeData Struct::typeData{Struct::initReflection};\
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pTypeOps = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {

/**
 * Used to specify a member inside a REFLECT_BEGIN/END pair
 **/
#define REFLECT_MEMBER(member)\
			{offsetof(XX, member), new Member_Internal<decltype(XX::member), XX>(#member, &XX::member)},

/**
 * Complete a type structure definition
 **/
#define REFLECT_END()\
		};\
		for (const eastl::pair<size_t, Member*>& mem : selfTypeData->members) { selfTypeData->memberOffsets[mem.second->name] = mem.first; }\
	}


/**
 *  Special version of the begin macro for types that are template specializations, such as Vec<float>
 **/
#define REFLECT_TEMPLATED_BEGIN(Struct)\
	TypeData Struct::typeData{Struct::initReflection};\
	template<>\
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pTypeOps = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {

/**
 * Special version of reflection function used for components. Just stores a member that tells you this type is a component
 * Intend to replace with proper type attributes at some point
 **/
#define REFLECT_COMPONENT_BEGIN(Struct)\
	TypeData Struct::typeData{Struct::initReflection};\
	void Struct::initReflection(TypeData* selfTypeData) {\
		using XX = Struct;\
		TypeDatabase::Data::Get().typeNames.emplace(#Struct, selfTypeData);\
		selfTypeData->id = Type::Index<XX>();\
		selfTypeData->isComponent = true;\
		selfTypeData->name = #Struct;\
		selfTypeData->size = sizeof(XX);\
		selfTypeData->pTypeOps = new TypeDataOps_Internal<XX>;\
		selfTypeData->members = {


// -----------------------------------
// ------------INTERNAL---------------
// -----------------------------------

// For storing lookup tables and other static type information
namespace TypeDatabase
{
	struct Data
	{
		static Data& Get() 
		{
			if (pInstance == nullptr)
				pInstance = new Data();
			return *pInstance;
		}
		eastl::map<eastl::string, TypeData*> typeNames;
		uint32_t typeCounter{ 0 };

	private:
		static Data* pInstance;
	};

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
}

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

template<typename T>
bool Member::IsType()
{
	// This returns true if you ask if the item is an unknown type, which is real bad
	return GetType() == TypeDatabase::Get<T>(); 
}

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