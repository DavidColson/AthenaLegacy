#include "Variant.h"

#include "Log.h"

Variant::Variant(const Variant& copy) : pInvoke(copy.pInvoke)
{
    pInvoke(VariantDataOperation::Clone, copy.pData, pData);
}

Variant::Variant(Variant&& copy) : pInvoke(copy.pInvoke)
{
    pInvoke(VariantDataOperation::Swap, copy.pData, pData);
    copy.pInvoke = &VariantDataPolicyNull::Invoke;
}

Variant& Variant::operator=(const Variant& copy)
{
    pInvoke(VariantDataOperation::Destroy, pData, ArgWrapper{});
    copy.pInvoke(VariantDataOperation::Clone, copy.pData, pData);
    pInvoke = copy.pInvoke;
    return *this;
}

Variant& Variant::operator=(Variant&& copy)
{
    pInvoke(VariantDataOperation::Destroy, pData, ArgWrapper{});
    copy.pInvoke(VariantDataOperation::Swap, copy.pData, pData);

    pInvoke = copy.pInvoke;
    copy.pInvoke = &VariantDataPolicyNull::Invoke;
    return *this;
}

Variant::~Variant()
{
    pInvoke(VariantDataOperation::Destroy, pData, ArgWrapper{});
}

TypeData& Variant::GetType()
{
    TypeData* pTypeData;
    pInvoke(VariantDataOperation::GetType, pData, pTypeData);
    return *pTypeData;
}