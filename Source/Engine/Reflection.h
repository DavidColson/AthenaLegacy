#pragma once

#include <string>
#include <unordered_map>

#define CAT(a, b) a##b

#define REGISTRATION									\
static void _register();								\
namespace { struct temp { temp() { _register(); } }; }	\
static const temp CAT(temp, __LINE__);									\
static void _register()

#define REFLECTABLE(CLASSNAME) static TypeData* GetTypeData() { return TypeDatabase::GetTypeData(#CLASSNAME); }

template<typename A>
struct Member;

struct Member_Base
{
	template<typename I, typename T>
	void set_value(I* instance, T value)
	{
		((Member<T I::*>*)this)->set_value(instance, value);
	}

	template<typename T, typename I>
	T get_value(I* instance)
	{
		return ((Member<T I::*>*)this)->get_value<T>(instance);
	}
};

template<typename A>
struct Member : public Member_Base
{
	A pPointer;

	Member(A pointer) { pPointer = pointer; }

	template<typename T, typename I>
	void set_value(I* obj, T value)
	{
		obj->*pPointer = value;
	}

	template<typename T, typename I>
	T& get_value(I* obj)
	{
		return obj->*pPointer;
	}
};

struct TypeData
{
	std::unordered_map<std::string, Member_Base*> m_memberList;

	template<typename A>
	TypeData* RegisterMember(std::string name, A accessor)
	{
		Member<A>* member = new Member<A>(accessor);
		m_memberList.insert({ name, member });
		return this;
	}
};

namespace TypeDatabase
{
	TypeData* GetTypeData(std::string typeName);
	TypeData* RegisterNewType(std::string typeName);
}