#pragma once

#include "ErrorHandling.h"

struct TypeData;

struct Variant
{
    template<typename T, typename Decayed = eastl::decay<T>::type>
    using DecayedIsNotVariant = eastl::enable_if_t<!eastl::is_same<Decayed, Variant>::value, Decayed>;

    // @Improvement, consider stack allocating a small amount of chars to use instead of always heap allocating, so small sized objects can use the stack 
    // Constructors and destructor
    Variant() {}
	Variant& operator=(const Variant& copy);
	Variant& operator=(Variant&& copy);
    ~Variant();

    // These enableif templates ensure that these constructors are used when the passed in type is not a variant, 
    // otherwise the variant copy constructor
    // accidentally gets used
    template <class T, typename Td = DecayedIsNotVariant<T>>
    Variant(T& val)
    {
        ASSERT(TypeDatabase::Get<Td>().size != 0, "Cannot store a variant to a type unknown to the type system");
        pTypeData = &TypeDatabase::Get<Td>();
        pData = new char[pTypeData->size];
        memcpy(pData, &val, pTypeData->size);
    }

    // basic idea, we have specialized versions of these constructors for types that are too big to fit into internal memory
    // are strings, arrays etc etc. that have custom constructors.
    template <class T, typename Td = DecayedIsNotVariant<T>>
    Variant(T&& rval)
    {
        // We need to use this variant data policy thing from rttr
        pTypeData = &TypeDatabase::Get<Td>();
        pData = new char[pTypeData->size];
        memcpy(pData, &rval, pTypeData->size);
    }

	Variant(const Variant& copy);
    Variant(Variant&& copy);

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