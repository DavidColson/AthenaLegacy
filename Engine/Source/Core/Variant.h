#pragma once

#include "ErrorHandling.h"

struct TypeData;

// For wrapping arbitrary arguments passed into Invoke below
// It just takes the address of whatever you gave it, and saves a void* pointer of it
struct ArgWrapper
{
    ArgWrapper() : pData(nullptr) {}

    template<typename T, typename = eastl::enable_if_t<!eastl::is_same<T, ArgWrapper>::value, T>>
    ArgWrapper(T&& data) : pData(reinterpret_cast<void*>(eastl::addressof(data))) {}

    template<typename T>
    T& Get() const
    {
        using raw_type = typename eastl::remove_reference<T>::type;
        return (*reinterpret_cast<raw_type*>(pData));
    }

    void* pData;
};

enum class VariantDataOperation
{
    Destroy,
    Clone,
    Swap,
    GetValue,
    GetType
};

typedef void* VariantData;

template<typename T, typename DerivedPolicy>
struct VariantDataPolicyBase
{
    static bool Invoke(VariantDataOperation operation, const VariantData& sourceVariantData, ArgWrapper argument)
    {
        switch (operation)
        {
        case VariantDataOperation::Clone:
        {
            DerivedPolicy::Clone(DerivedPolicy::GetValue(sourceVariantData), argument.Get<VariantData>());
            break;
        }
        case VariantDataOperation::Destroy:
        {
            DerivedPolicy::Destroy(const_cast<T&>(DerivedPolicy::GetValue(sourceVariantData)));
            break;
        }
        case VariantDataOperation::Swap:
        {
            DerivedPolicy::Swap(const_cast<T&>(DerivedPolicy::GetValue(sourceVariantData)), argument.Get<VariantData>());
            break;
        }
        case VariantDataOperation::GetValue:
        {
            argument.Get<const void*>() = &DerivedPolicy::GetValue(sourceVariantData);
            break;
        }
        case VariantDataOperation::GetType:
        {
            argument.Get<TypeData*>() = &TypeDatabase::Get<T>();
            break;
        }
        default:
            break;
        }
        return true;
    }
};

using VariantDataPolicyFunc = bool (*)(VariantDataOperation, const VariantData&, ArgWrapper);

template<typename T>
struct VariantDataPolicyNormal : VariantDataPolicyBase<T, VariantDataPolicyNormal<T>>
{
    template<typename U>
    static void Create(U&& rval, VariantData& destination)
    {   
        // Use rvalue to construct a new instance at destination
        reinterpret_cast<T*&>(destination)  = new T(eastl::forward<U>(rval));
    }

    static const T& GetValue(const VariantData& variantData)
    {
        return *reinterpret_cast<T*const &>(variantData);
    }

    static void Clone(const T& value, VariantData& destination)
    {   
        // call new on the type copying existing value
        reinterpret_cast<T*&>(destination) = new T(value);
    }

    static void Swap(T& value, VariantData& destination)
    {   
        // Set our variant data to the address of the existing value
        reinterpret_cast<T*&>(destination) = &value;
    }

    static void Destroy(T& value)
    {
        delete &value;
    }
};

struct VariantDataPolicyNull
{
    static bool Invoke(VariantDataOperation operation, const VariantData& sourceVariantData, ArgWrapper argument)
    {
        switch (operation)
        {
        case VariantDataOperation::Clone:
        case VariantDataOperation::Destroy:
        case VariantDataOperation::Swap:
            break;
        case VariantDataOperation::GetValue:
        {
            argument.Get<const void*>() = nullptr;
            break;
        }
        default:
            break;
        }
        return true;
    }

    template<typename U>
    static void Create(U&& rval, VariantData& destination)
    {   
    }
};

struct Variant
{
    template<typename T, typename Decayed = eastl::decay<T>::type>
    using DecayedIsNotVariant = eastl::enable_if_t<!eastl::is_same<Decayed, Variant>::value, Decayed>;

    // @Improvement, consider stack allocating a small amount of chars to use instead of always heap allocating, so small sized objects can use the stack 
    // Constructors and destructor
    Variant() : pInvoke(&VariantDataPolicyNull::Invoke) {}
	Variant& operator=(const Variant& copy);
	Variant& operator=(Variant&& copy);
    ~Variant();


    template <class T, typename DecayedType = DecayedIsNotVariant<T>>
    Variant(T&& rval) : pInvoke(&VariantDataPolicyNormal<DecayedType>::Invoke)
    {
        VariantDataPolicyNormal<DecayedType>::Create(eastl::forward<T>(rval), pData);
    }

	Variant(const Variant& copy);
    Variant(Variant&& copy);

    // Getters
    template<typename T>
    T& GetValue()
    {
        const void* value;
        pInvoke(VariantDataOperation::GetValue, pData, value);
        return *reinterpret_cast<T*>(const_cast<void*>(value));
    }

    TypeData& GetType();

    VariantDataPolicyFunc pInvoke;
    void* pData{ nullptr };
};