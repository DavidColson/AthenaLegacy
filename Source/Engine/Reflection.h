#pragma once

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

// Adds a convienience function to structs and classes allowing you to retrieve the type from within a class
#define REFLECTABLE(CLASSNAME) static TypeDatabase::Type* GetType() { return TypeDatabase::GetType<CLASSNAME>(); }

// Convienience macro for registering a type without specifying a string
#define RegisterNewType(Type) TypeDatabase::Detail::RegisterNewType_Internal<Type>(#Type)

typedef uintptr_t TypeId;

namespace TypeDatabase
{
	struct Member;
	template<typename t, typename a>
	struct Member_Internal;


	// Creates unique id numbers for all instances of the template (for each type it's used with
	template<typename T>
	struct TypeIdGenerator
	{
	public:
		static TypeId Id() { return reinterpret_cast<TypeId>(&Id); }
	};


	// The core of the reflection system is the type
	// It represents any kind of type, from basic ints to classes
	// It stores the id, name and a map of members in the type
	struct Type
	{
		Type() : m_name(""), m_id(0) {}
		Type(const char* name, TypeId id) : m_name(name), m_id(id) {}

		template<typename T, typename I>
		Type* RegisterMember(const char* name, T I::*accessor)
		{
			Detail::Member_Internal<T I::*>* member = new Detail::Member_Internal<T I::*>(accessor);
			member->m_type = TypeDatabase::GetType<T>();
			m_memberList.insert({ name, member });
			return this;
		}

		bool operator==(const Type* other)
		{
			return this->m_id == other->m_id;
		}

		Member* GetMember(const char* name)
		{
			assert(m_memberList.count(name) == 1); // The member you're trying to access doesn't exist
			return m_memberList[name];
		}

		const char* m_name;
		TypeId m_id;
		std::unordered_map <std::string, Member* > m_memberList;
	};




	// Represents a specific member of a class. 
	// Contains the type of that member, plus an accessor to read or change the member value of some instance
	struct Member
	{
		Type* m_type;

		template<typename I, typename T>
		void SetValue(I& instance, T value)
		{
			((Detail::Member_Internal<T I::*>*)this)->SetValue(instance, value);
		}

		template<typename T, typename I>
		T GetValue(I& instance)
		{
			return ((Detail::Member_Internal<T I::*>*)this)->GetValue<T>(instance);
		}
	};




	// Both versions of GetType allow you to retrieve the type object from either the type itself, or an instance of the type
	template<typename T>
	Type* GetType()
	{
		TypeId id = TypeIdGenerator<T>::Id();
		assert(Detail::typeDatabase.count(id) == 1); // The type you are querying does not exist in the database, please register it
		return &Detail::typeDatabase[id];
	}

	template<typename T>
	Type* GetType(T& obj)
	{
		TypeId id = TypeIdGenerator<T>::Id();
		assert(Detail::typeDatabase.count(id) == 1); // The type you are querying does not exist in the database, please register it
		return &Detail::typeDatabase[id];
	}

	





	namespace Detail
	{
		// Actual storage of type information
		extern std::unordered_map<TypeId, Type> typeDatabase;

		// Registers a new type to the database
		template<typename T>
		Type* RegisterNewType_Internal(const char* typeName)
		{
			TypeId id = TypeIdGenerator<T>::Id();
			Detail::typeDatabase.emplace(id, Type(typeName, id));
			return &Detail::typeDatabase[id];
		}

		// Internal representation of members (it's a subclass so Member doesn't need to be a template)
		// Stores the accessor for members and sets up the internal type value
		template<typename A>
		struct Member_Internal : public Member
		{
			A m_pPointer;

			Member_Internal(A pointer) { m_pPointer = pointer;  }

			template<typename T, typename I>
			void SetValue(I& obj, T value)
			{
				obj.*m_pPointer = value;
			}

			template<typename T, typename I>
			T& GetValue(I& obj)
			{
				return obj.*m_pPointer;
			}
		};
	}
}
