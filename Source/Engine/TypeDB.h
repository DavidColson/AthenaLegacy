#pragma once


#include <string>
#include <unordered_map>

#include "Log.h"
#include "ErrorHandling.h"
// #TodoTypeDB Consider inlining a lot of these smaller funtions

namespace reg_helper
{
	template <typename T>
	inline int call(void(*f)())
	{
		static const int s = [&f]() {
			f();
			return 0;
		}();
		return s;
	}
}

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REGISTER_EXTERN(TYPE)\
	template <typename T>\
	extern void register_func();\
	template <>\
	void register_func<TYPE>();\
	static const int CAT(TYPE, __LINE__) =\
		reg_helper::call<TYPE>(&register_func<TYPE>)

#define REGISTER(TYPE)\
	template <> \
	void register_func<TYPE>()

// Convienience macro for registering a type without specifying a string
#define NewType(Type) TypeDB::Detail::RegisterNewType_Internal<Type>(#Type)
#define NewTypeAsComponent(Type) ComponentIdToTypeIdMap::Get()->AddRelation(TypeDB::TypeIdGenerator<Type>::Id(), GetComponentId<Type>()); TypeDB::Detail::RegisterNewType_Internal<Type>(#Type)

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
		inline T& Get();

		template<typename T>
		inline bool IsA();

		Type* m_type;
		void* m_data;
	};

	struct Variant : public VariantBase
	{
		inline Variant() {}
		inline ~Variant();

		template<typename T>
		inline Variant(const T& value);

		inline Variant(const Variant& copy);

		template<typename T>
		inline Variant& operator=(const T& value);

		inline Variant& operator=(const Variant& value);
	};

	struct RefVariant : VariantBase
	{
		template<typename T>
		inline RefVariant(const T& value);

		inline RefVariant(const VariantBase& copy);

		inline RefVariant(const Variant& copy);

		inline RefVariant(const RefVariant& copy);

		template<typename T>
		inline RefVariant& operator=(const T& value);
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

		// #TodoTypeDB Add GetMemberValue<T> and SetMemberValue functions for convenience (also GetMemberType)

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

		Type* GetType();
		
		template<typename T> 
		bool IsType();

		bool IsType(std::string typeName);

		virtual void SetValue(RefVariant&& obj, RefVariant value) const = 0;

		template<typename T>
		T GetValue(RefVariant& obj) const;

		template<typename T>
		T GetValue(RefVariant&& obj) const;

		virtual Variant GetValue(RefVariant& obj) const = 0;

		virtual Variant GetValue(RefVariant&& obj) const = 0;

		template<typename T>
		T& GetRefValue(RefVariant& obj) const;

		template<typename T>
		T& GetRefValue(RefVariant&& obj) const;

		virtual RefVariant GetRefValue(RefVariant& obj) const = 0;

		virtual RefVariant GetRefValue(RefVariant&& obj) const = 0;
	};

	Type* GetTypeFromString(std::string typeName);

	template<typename T>
	Type* GetType();

	template<typename T>
	Type* GetType(T& obj);

	Type* GetType(TypeId typeId);

	void RegisterBaseTypes();

	namespace Detail
	{
		struct Data
		{
			static Data& Get() 
			{
				if (pInstance == nullptr)
					pInstance = new Data();
				return *pInstance;
			}
			static Data* pInstance;

			// Actual storage of type information
			std::unordered_map<TypeId, Type> typeDatabase;
			std::unordered_map<std::string, TypeId> typeNames;
		};

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

			inline virtual void SetValue(RefVariant&& obj, RefVariant value) const override
			{
				// #TodoTypeDB Get the type that value actually is and print it's name as an error i.e. 'value is a float but we expected a vec2'
				ASSERT(value.IsA<T>(), "The value you supplied isn't the correct type");
				ASSERT(obj.IsA<I>(), "The instance you supplied isn't the correct type");
				obj.Get<I>().*m_pPointer = value.Get<T>();
			}

			inline virtual Variant GetValue(RefVariant&& obj) const override
			{
				ASSERT(obj.IsA<I>(), "The instance you supplied isn't the correct type");
				return Variant(obj.Get<I>().*m_pPointer);
			}
			inline virtual Variant GetValue(RefVariant& obj) const override
			{
				ASSERT(obj.IsA<I>(), "The instance you supplied isn't the correct type");
				return Variant(obj.Get<I>().*m_pPointer);
			}

			inline virtual RefVariant GetRefValue(RefVariant& obj) const override
			{
				ASSERT(obj.IsA<I>(), "The instance you supplied isn't the correct type");
				return RefVariant(obj.Get<I>().*m_pPointer);
			}
			inline virtual RefVariant GetRefValue(RefVariant&& obj) const override
			{
				ASSERT(obj.IsA<I>(), "The instance you supplied isn't the correct type");
				return RefVariant(obj.Get<I>().*m_pPointer);
			}
		};
	}




	// *******************************
	// IMPLEMENTATIONS ***************
	//********************************

	// VariantBase
	//////////////

	template<typename T>
	inline T& VariantBase::Get()
	{
		ASSERT(IsA<T>(), "Incorrect Type passed to variant Get");
		return *reinterpret_cast<T*>(m_data);
	}

	template<typename T>
	inline bool VariantBase::IsA()
	{
		return TypeIdGenerator<T>::Id() == m_type->m_id;
	}





	// Variant
	//////////

	inline Variant::~Variant()
	{
		delete[] reinterpret_cast<char *>(m_data);
	}

	inline Variant::Variant(const Variant& copy)
	{
		m_type = copy.m_type;
		m_data = new char[m_type->m_size];
		memcpy(m_data, copy.m_data, m_type->m_size);
	}

	template<typename T>
	inline Variant::Variant(const T& value)
	{
		size_t size = TypeDB::GetType<T>()->m_size;
		m_data = new char[size];
		m_type = TypeDB::GetType<T>();
		memcpy(m_data, &value, size);
	}

	template<typename T>
	inline Variant& Variant::operator=(const T& value)
	{
		if (m_type->m_id != TypeIdGenerator<T>::Id() && m_type->m_id != 0)
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
		return *this;
	}

	inline Variant& Variant::operator=(const Variant& value)
	{
		m_type = value.m_type;
		m_data = new char[m_type->m_size];
		memcpy(m_data, value.m_data, m_type->m_size);
		return *this;
	}



	// RefVariant
	/////////////

	inline RefVariant::RefVariant(const VariantBase& copy)
	{
		m_type = copy.m_type;
		m_data = copy.m_data;
	}

	inline RefVariant::RefVariant(const Variant& copy)
	{
		m_type = copy.m_type;
		m_data = copy.m_data;
	}

	inline RefVariant::RefVariant(const RefVariant& copy)
	{
		m_type = copy.m_type;
		m_data = copy.m_data;
	}

	template<typename T>
	inline RefVariant::RefVariant(const T& value)
	{
		m_type = TypeDB::GetType<T>();
		m_data = const_cast<T*>(&value);
	}

	template<typename T>
	inline RefVariant& RefVariant::operator=(const T& value)
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


	// Member
	/////////

	template<typename T>
	bool TypeDB::Member::IsType()
	{
		TypeId testId = TypeIdGenerator<T>::Id();
		return testId == m_type->m_id;
	}

	template<typename T>
	T TypeDB::Member::GetValue(RefVariant& obj) const
	{
		return GetValue(obj).Get<T>();
	}
	template<typename T>
	T TypeDB::Member::GetValue(RefVariant&& obj) const
	{
		return GetValue(obj).Get<T>();
	}

	template<typename T>
	T& TypeDB::Member::GetRefValue(RefVariant& obj) const
	{
		return GetRefValue(obj).Get<T>();
	}
	template<typename T>
	T& TypeDB::Member::GetRefValue(RefVariant&& obj) const
	{
		return GetRefValue(obj).Get<T>();
	}

	// TypeDB
	/////////


	template<typename T>
	Type* TypeDB::GetType()
	{
		TypeId id = TypeIdGenerator<T>::Id();
		ASSERT(Detail::Data::Get().typeDatabase.count(id) == 1, "The type you are querying does not exist in the database, please register it");
		return &Detail::Data::Get().typeDatabase[id];
	}

	template<typename T>
	Type* TypeDB::GetType(T& obj)
	{
		TypeId id = TypeIdGenerator<T>::Id();
		ASSERT(Detail::Data::Get().typeDatabase.count(id) == 1, "The type you are querying does not exist in the database, please register it");
		return &Detail::Data::Get().typeDatabase[id];
	}




	// TypeDB::Detail
	/////////////////

	template<typename T>
	Type* Detail::RegisterNewType_Internal(const char* typeName)
	{
		TypeId id = TypeIdGenerator<T>::Id();
		Detail::Data::Get().typeDatabase.emplace(id, Type(typeName, id, sizeof(T), new Constructor_Internal<T>()));
		Detail::Data::Get().typeNames.emplace(typeName, id);
		return &Detail::Data::Get().typeDatabase[id];
	}
}