#include "Variant.h"

#include "Log.h"

Variant::Variant(const Variant& copy)
{
    pTypeData = copy.pTypeData;
    pData = new char[pTypeData->size];
    memcpy(pData, copy.pData, pTypeData->size);
}

Variant::Variant(Variant&& copy)
{
    pTypeData = copy.pTypeData;
    copy.pTypeData = nullptr;

    pData = copy.pData;
    copy.pData = nullptr;
}

Variant& Variant::operator=(const Variant& copy)
{
    pTypeData = copy.pTypeData;
    pData = new char[pTypeData->size];
    memcpy(pData, copy.pData, pTypeData->size);
    return *this;
}

Variant& Variant::operator=(Variant&& copy)
{
    pTypeData = copy.pTypeData;
    copy.pTypeData = nullptr;

    pData = copy.pData;
    copy.pData = nullptr;
    return *this;
}

Variant::~Variant()
{
    if (pData)
    {
        delete[] reinterpret_cast<char*>(pData);
        pTypeData = nullptr;
    }
}

TypeData& Variant::GetType()
{
    return *pTypeData;
}