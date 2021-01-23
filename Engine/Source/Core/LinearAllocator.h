#pragma once

struct LinearAllocator
{
    LinearAllocator(size_t initSize);

    ~LinearAllocator();

    // Allocate nBytes of data on the stack. Alignment must be power of 2
    void* Allocate(size_t nBytes, size_t alignment = 0);

    void Clear();

    char* pData{ nullptr };
    size_t totalSize{ 0 };
    size_t offset{ 0 };
};