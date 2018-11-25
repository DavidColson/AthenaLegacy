#pragma once

#include "Log.h"

#include <string>
#include <unordered_map>
#include <assert.h>

// CONVIENIENCE MACROS
#define CAT(a, b) a##b

// This is used to create a function that'll be called before main() for registering types into the type database
#define REGISTRATION									\
static void _register();								\
namespace { struct temp { temp() { _register(); } }; }	\
static const temp CAT(temp, __LINE__);					\
static void _register()

// Convienience macro for registering a type without specifying a string
#define RegisterNewType(Type) TypeDB::Detail::RegisterNewType_Internal<Type>(#Type)
#define RegisterNewTypeAsComponent(Type) g_componentTypeMap.AddRelation(TypeIdGenerator<Type>::Id(), GetComponentId<Type>()); TypeDB::Detail::RegisterNewType_Internal<Type>(#Type)

typedef uintptr_t TypeId;

namespace TypeDB
{
	struct Member;
	template<typename t, typename a>
	struct Member_Internal;
	struct Type;


	// Creates unique id numbers for all instances of the template (for each type it's used with
	template<typename T>
	struct TypeIdGenerator
	{
	public:
		static TypeId Id() { return reinterpret_cast<TypeId>(&Id); }
	};


	struct VariantBase
	{
		template<typename T>
		T& Get();

		template<typename T>
		bool IsA();

		Type* m_type;
		void* m_data;
	};

	struct Variant : public VariantBase
	{
		Variant() {}
		~Variant();

		template<typename T>
		Variant(const T& value);

		Variant(const Variant& copy);

		template<typename T>
		Variant& operator=(const T& value);
	};

	struct RefVariant : VariantBase
	{
		template<typename T>
		RefVariant(const T& value);

		RefVariant(const VariantBase& copy);

		RefVariant(const Variant& copy);

		RefVariant(const RefVariant& copy);

		template<typename T>
		RefVariant& operator=(const T& value);
	};
	



	struct Constructor
	{
		virtual Variant Invoke() = 0;
	};


	// The core of the reflection system is the type
	// It represents any kind of type, from basic ints to classes
	// It stores the id, name and a map of members in the type
	struct Type
	{
		Type() : m_name(""), m_id(0), m_size(0) {}
		Type(const char* name, TypeId id, size_t size, Constructor* con) : m_name(name), m_id(id), m_size(size), m_constructor(con) {}

		template<typename T, typename I>
		Type* RegisterMember(const char* name, T I::*accessor);

		bool operator==(const Type* other);

		Member* GetMember(const char* name);

		Variant New();

		const char* m_name;
		TypeId m_id;
		size_t m_size;
		Constructor* m_constructor;
		std::unordered_map <std::string, Member* > m_memberList;
	};






	// Represents a specific member of a class. 
	// Contains the type of that member, plus an accessor to read or change the member value of some instance
	struct Member
	{
		Type* m_type;

		virtual void SetValue(RefVariant&& obj, RefVariant value) = 0;

		virtual Variant GetValue(RefVariant&& obj) = 0;
		virtual Variant GetValue(RefVariant& obj) = 0;

		virtual RefVariant GetRefValue(RefVariant& obj) = 0;
		virtual RefVariant GetRefValue(RefVariant&& obj) = 0;
	};

	Type* GetTypeFromString(std::string typeName);

	template<typename T>
	Type* GetType();

	template<typename T>
	Type* GetType(T& obj);

	Type* GetType(TypeId typeId);

	namespace Detail
	{
		// Actual storage of type information
		extern std::unordered_map<TypeId, Type> typeDatabase;
		extern std::unordered_map<std::string, TypeId> typeNames;

		// Registers a new type to the database
		template<typename T>
		Type* RegisterNewType_Internal(const char* typeName);

		template<typename T>
		struct Constructor_Internal : public Constructor
		{
			virtual Variant Invoke() override
			{
				return *(new T());
			}
		};

		// Internal representation of members (it's a subclass so Member doesn't need to be a template)
		// Stores the accessor for members and sets up the internal type value
		template<typename T, typename I>
		struct Member_Internal : public Member
		{
			T I::* m_pPointer;

			Member_Internal(T I::* pointer) { m_pPointer = pointer;  }

			virtual void SetValue(RefVariant&& obj, RefVariant value) override
			{
				assert(value.IsA<T>()); // The value you supplied isn't the correct type TODO: Get the type that value actually is and print it's name as an error i.e. "value is a float but we expected a vec2"
				assert(obj.IsA<I>()); // The instance you supplied isn't the correct type
				obj.Get<I>().*m_pPointer = value.Get<T>();
			}

			virtual Variant GetValue(RefVariant&& obj) override
			{
				assert(obj.IsA<I>()); // The instance you supplied isn't the correct type
				return Variant(obj.Get<I>().*m_pPointer);
			}
			virtual Variant GetValue(RefVariant& obj) override
			{
				assert(obj.IsA<I>()); // The instance you supplied isn't the correct type
				return Variant(obj.Get<I>().*m_pPointer);
			}

			virtual RefVariant GetRefValue(RefVariant& obj)
			{
				assert(obj.IsA<I>()); // The instance you supplied isn't the correct type
				return RefVariant(obj.Get<I>().*m_pPointer);
			}
			virtual RefVariant GetRefValue(RefVariant&& obj)
			{
				assert(obj.IsA<I>()); // The instance you supplied isn't the correct type
				return RefVariant(obj.Get<I>().*m_pPointer);
			}
		};
	}
}


// *******************************
// IMPLEMENTATIONS ***************
//********************************

using namespace TypeDB;




// VariantBase
//////////////

template<typename T>
T& VariantBase::Get()
{
	return *reinterpret_cast<T*>(m_data);
}

template<typename T>
bool VariantBase::IsA()
{
	return TypeIdGenerator<T>::Id() == m_type->m_id;
}





// Variant
//////////

template<typename T>
Variant::Variant(const T& value)
{
	size_t size = TypeDB::GetType<T>()->m_size;
	m_data = new char[size];
	m_type = TypeDB::GetType<T>();
	memcpy(m_data, &value, size);
}

template<typename T>
Variant& Variant::operator=(const T& value)
{
	if (m_typeId != TypeIdGenerator<T>::Id() && m_typeId != 0)
	{
		delete[] reinterpret_cast<char *>(m_data);

		size_t size = TypeDB::GetType<T>()->m_size;
		m_data = new char[size];
		m_type = TypeDB::GetType<T>();
		memcpy(m_data, &value, size);
	}
	else
	{
		size_t size = TypeDB::GetType<T>()->m_size;
		memcpy(m_data, &value, size);
	}
}




// RefVariant
/////////////

template<typename T>
RefVariant::RefVariant(const T& value)
{
	m_type = TypeDB::GetType<T>();
	m_data = const_cast<T*>(&value);
}

template<typename T>
RefVariant& RefVariant::operator=(const T& value)
{
	m_type = TypeDB::GetType<T>();
	m_data = const_cast<T*>(&value);
	return *this;
}




// Type
///////

template<typename T, typename I>
Type* Type::RegisterMember(const char* name, T I::*accessor)
{
	Detail::Member_Internal<T, I>* member = new Detail::Member_Internal<T, I>(accessor);
	member->m_type = TypeDB::GetType<T>();
	m_memberList.insert({ name, member });
	return this;
}




// TypeDB
/////////


template<typename T>
Type* TypeDB::GetType()
{
	TypeId id = TypeIdGenerator<T>::Id();
	assert(Detail::typeDatabase.count(id) == 1); // The type you are querying does not exist in the database, please register it
	return &Detail::typeDatabase[id];
}

template<typename T>
Type* TypeDB::GetType(T& obj)
{
	TypeId id = TypeIdGenerator<T>::Id();
	assert(Detail::typeDatabase.count(id) == 1); // The type you are querying does not exist in the database, please register it
	return &Detail::typeDatabase[id];
}




// TypeDB::Detail
/////////////////

template<typename T>
Type* Detail::RegisterNewType_Internal(const char* typeName)
{
	TypeId id = TypeIdGenerator<T>::Id();
	Detail::typeDatabase.emplace(id, Type(typeName, id, sizeof(T), new Constructor_Internal<T>()));
	Detail::typeNames.emplace(typeName, id);
	return &Detail::typeDatabase[id];
}