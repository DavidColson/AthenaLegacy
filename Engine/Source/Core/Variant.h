#pragma once

#include "ErrorHandling.h"

struct TypeData;

struct Variant
{
    // @Improvement, consider stack allocating a small amount of chars to use instead of always heap allocating, so small sized objects can use the stack 
    // Constructors and destructor
    Variant() {}
	Variant(const Variant& copy);
    Variant(Variant&& copy);
	Variant& operator=(const Variant& copy);
	Variant& operator=(Variant&& copy);
    ~Variant();

    // These enableif templates ensure that these constructors are used when the passed in type is not a variant, 
    // otherwise the variant copy constructor
    // accidentally gets used
    template <class T>
    Variant(T& val, typename eastl::enable_if<!eastl::is_same<typename eastl::decay<T>::type, Variant>::value>::type* = 0)
    {
        ASSERT(TypeDatabase::Get<T>().size != 0, "Cannot store a variant to a type unknown to the type system");
        pTypeData = &TypeDatabase::Get<T>();
        pData = new char[pTypeData->size];
        memcpy(pData, &val, pTypeData->size);
    }


    // basic idea, we have specialized versions of these constructors for types that are too big to fit into internal memory
    // are strings, arrays etc etc. that have custom constructors.
    template <class T>
    Variant(T&& rval, typename eastl::enable_if<!eastl::is_same<typename eastl::decay<T>::type, Variant>::value>::type* = 0)
    {
        // is copy constructible?
        // We need to use this variant data policy thing from rttr
        pTypeData = &TypeDatabase::Get<T>();
        pData = new char[pTypeData->size];
        memcpy(pData, &rval, pTypeData->size);
    }

    // Getters
    template<typename T>
    T& GetValue()
    {
        ASSERT(*pTypeData == TypeDatabase::Get<T>(), "Cannot get value of this variant, underlying data does not match requested type.");
        return *reinterpret_cast<T*>(pData);
    }

    TypeData& GetType();

    void* pData{ nullptr };
    TypeData* pTypeData{nullptr};
};