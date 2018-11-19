#pragma once

#include <string>
#include <unordered_map>
#include <assert.h>
#include "Log.h"

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




	struct Variant
	{
		Variant() {}

		template<typename T>
		Variant(T inValue) : m_impl(new VariantImpl<T>(inValue)), m_typeId(TypeDatabase::TypeIdGenerator<T>::Id()) {}

		// Copy constructor
		Variant(const Variant& other)  
		{
			m_impl = other.m_impl->Clone();
			m_typeId = other.m_typeId;
		}

		~Variant() { if (m_typeId != 0) delete m_impl; Log::Print(Log::EMsg, "Deleting Variant"); }

		template<typename T>
		Variant& operator=(T value)
		{
			if (TypeDatabase::TypeIdGenerator<T>::Id() != m_typeId && m_typeId != 0)
				delete m_impl;
			m_impl = new VariantImpl<T>(value);
			m_typeId = TypeDatabase::TypeIdGenerator<T>::Id();
			return *this;
		}

		template<typename T>
		T& Get() const
		{
			assert(m_typeId == TypeDatabase::TypeIdGenerator<T>::Id()); // You're trying to get a data type from a variant that doesn't hold that datatype
			return static_cast<VariantImpl<T>*>(m_impl)->GetValue();
		}

		struct AbstractVariantImpl
		{
			virtual ~AbstractVariantImpl() {}
			virtual AbstractVariantImpl* Clone() = 0;
		};

		template<typename T>
		struct VariantImpl : public AbstractVariantImpl
		{
			VariantImpl(T inValue) : m_value(inValue) {}
			~VariantImpl() {}
			virtual AbstractVariantImpl* Clone() { return new VariantImpl<T>(m_value);  }
			T& GetValue() { return m_value; }
			T m_value;
		};

		AbstractVariantImpl* m_impl;
		TypeId m_typeId{ 0 };
	};


	// Make a ref variant
	// Ref variant is basically a wrapper around a pointer
	// Data is now not a new VariantImpl, but rather a const_cast<T *> into a pointer.
	// Data stores a pointer to a 



	// The core of the reflection system is the type
	// It represents any kind of type, from basic ints to classes
	// It stores the id, name and a map of members in the type
	struct Type
	{
		Type() : m_name(""), m_id(0), m_size(0) {}
		Type(const char* name, TypeId id, size_t size) : m_name(name), m_id(id), m_size(size) {}

		template<typename T, typename I>
		Type* RegisterMember(const char* name, T I::*accessor)
		{
			Detail::Member_Internal<T, I>* member = new Detail::Member_Internal<T, I>(accessor);
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
		size_t m_size;
		std::unordered_map <std::string, Member* > m_memberList;
	};






	// Represents a specific member of a class. 
	// Contains the type of that member, plus an accessor to read or change the member value of some instance
	struct Member
	{
		Type* m_type;

		virtual void SetValue(Variant& instance, Variant value) = 0;

		virtual Variant GetValue(Variant& instance) = 0;
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
			Detail::typeDatabase.emplace(id, Type(typeName, id, sizeof(T)));
			return &Detail::typeDatabase[id];
		}

		// Internal representation of members (it's a subclass so Member doesn't need to be a template)
		// Stores the accessor for members and sets up the internal type value
		template<typename T, typename I>
		struct Member_Internal : public Member
		{
			T I::* m_pPointer;

			Member_Internal(T I::* pointer) { m_pPointer = pointer;  }

			virtual void SetValue(Variant& obj, Variant value) override
			{
				// TODO assert if value.is_a<T> == false
				obj.Get<I>().*m_pPointer = value.Get<T>();
			}

			virtual Variant GetValue(Variant& obj) override
			{
				return Variant(obj.Get<I>().*m_pPointer);
			}
		};
	}
}
