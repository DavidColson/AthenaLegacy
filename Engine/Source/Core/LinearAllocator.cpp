
#include "LinearAllocator.h"
#include "ErrorHandling.h"

LinearAllocator::LinearAllocator(size_t initSize)
{
    pData = new char[initSize];
}

LinearAllocator::~LinearAllocator()
{
    // memfree whole blockP
    delete[] pData;
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

    // move head pointer to current + nBytes + padding
    
    uintptr_t nextAddress = currentAddress + padding; 
    offset += nBytes + padding;
    return reinterpret_cast<void*>(nextAddress);
}

void LinearAllocator::Clear()
{
    // set head pointer to base
    offset = 0;
}