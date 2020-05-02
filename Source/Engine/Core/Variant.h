#pragma once

#include "ErrorHandling.h"

struct TypeData;

class Variant
{
    // @Improvement, consider stack allocating a small amount of chars to use instead of always heap allocating, so small sized objects can use the stack
public:
    
    // Constructors and destructor
    Variant() {}
	Variant(const Variant& copy);
    Variant(Variant&& copy);
	Variant& operator=(const Variant& copy);
	Variant& operator=(Variant&& copy);
    ~Variant();

    template <class T>
    Variant(T& val, typename eastl::enable_if<!eastl::is_same<typename eastl::decay<T>::type, Variant>::value>::type* = 0)
    {
        ASSERT(TypeDatabase::Get<T>().size != 0, "Cannot store a variant to a type unknown to the type system");
        pTypeData = &TypeDatabase::Get<T>();
        pData = new char[pTypeData->size];
        memcpy(pData, &val, pTypeData->size);
    }

   template <class T>
    Variant(T&& rval, typename eastl::enable_if<!eastl::is_same<typename eastl::decay<T>::type, Variant>::value>::type* = 0)
    {
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

private:
    void* pData{ nullptr };
    TypeData* pTypeData{nullptr};
};