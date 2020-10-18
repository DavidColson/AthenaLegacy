#pragma once

#include "ErrorHandling.h"

#include "EASTL/iterator.h"

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


// Custom Type Traits
/////////////////////////


// Note that eastl::begin returns an iterator to the first element, which we dereference and give to decltype
template<typename T, typename Enable = void>
struct ArrayElementType { using type = T; };
template<typename T>
struct ArrayElementType<T, eastl::enable_if_t<eastl::is_array<T>::value>>
{
    using type = eastl::decay_t<decltype(*eastl::begin(eastl::declval<T&>()))>;
};
template<typename T>
using ArrayElementType_t = typename ArrayElementType<T>::type;

template<typename T>
using IsCharArray = eastl::integral_constant<bool, eastl::is_same<char, ArrayElementType_t<T>>::value>;
// using IsCharArray = eastl::integral_constant<bool, eastl::is_array<T>::value && eastl::is_same<char, ArrayElementType<T>>::value && (eastl::rank<T>::value == 1)>;



template<typename T>
struct DecayExceptArray
{
    using U = eastl::remove_reference_t<T>;

    using type = eastl::conditional_t< 
            eastl::is_function<U>::value,
            typename eastl::add_pointer<U>::type,
            typename eastl::remove_cv<U>::type
            >;
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

struct VariantDataPolicyString : public VariantDataPolicyNormal<eastl::string>
{
    template<typename U>
    static void Create(U&& rval, VariantData& destination)
    {
        reinterpret_cast<eastl::string*&>(destination)  = new eastl::string(eastl::forward<U>(rval));
    }

    template<size_t N>
    static void Create(const char (&val)[N], VariantData& destination)
    {
        reinterpret_cast<eastl::string*&>(destination)  = new eastl::string(val, N - 1);
    }
};

template<typename T>
using VariantDataPolicy = eastl::conditional_t  <eastl::is_same<T, eastl::string>::value || IsCharArray<T>::value,
                                                    VariantDataPolicyString,
                                                    VariantDataPolicyNormal<T>
                                                >;


struct Variant
{
    template<typename T, typename Decayed = DecayExceptArray<T>::type>
    using DecayedIsNotVariant = eastl::enable_if_t<!eastl::is_same<Decayed, Variant>::value, Decayed>;

    // @Improvement, consider stack allocating a small amount of chars to use instead of always heap allocating, so small sized objects can use the stack 
    // Constructors and destructor
    Variant() : pInvoke(&VariantDataPolicyNull::Invoke) {}
	Variant& operator=(const Variant& copy);
	Variant& operator=(Variant&& copy);
    ~Variant();


    template <class T, typename DecayedType = DecayedIsNotVariant<T>>
    Variant(T&& rval) : pInvoke(&VariantDataPolicy<DecayedType>::Invoke)
    {
        //TODO: error if attempting to create a variant from a type not in the type database
        VariantDataPolicy<DecayedType>::Create(eastl::forward<T>(rval), pData);
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