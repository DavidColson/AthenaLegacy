
#include "LinearAllocator.h"
#include "ErrorHandling.h"
#include "Log.h"

LinearAllocator::LinearAllocator()
{}

LinearAllocator::~LinearAllocator()
{
    delete[] pData;
}

void LinearAllocator::Init(size_t size)
{
    pData = new char[size];
    totalSize = size;
}

uintptr_t AlignAddress(uintptr_t address, size_t alignment)
{
    const size_t mask = alignment - 1;
    ASSERT((alignment & mask) == 0, "Alignment must be a power of 2");
    return (address + mask) & ~mask;
}

void* LinearAllocator::Allocate(size_t nBytes, size_t alignment)
{
    uintptr_t currentAddress = reinterpret_cast<uintptr_t>(pData) + offset;

    // Calculate alignment
    size_t padding = 0;
    if (alignment != 0)
        padding = AlignAddress(currentAddress, alignment) - currentAddress;

    ASSERT(offset + padding + nBytes < totalSize, "Memory buffer overflow");

    uintptr_t nextAddress = currentAddress + padding; 
    offset += nBytes + padding;
    return reinterpret_cast<void*>(nextAddress);
}

void LinearAllocator::Clear()
{
    // set head pointer to base
    offset = 0;
}