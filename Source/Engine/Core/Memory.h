#pragma once


#include "Log.h"

// EASTL expects us to define these, see allocator.h line 194
void* operator new[](size_t size, const char* pName, int /* flags */,
	unsigned /* debugFlags */, const char* file, int line)
{
	void* newStuff = malloc(size);
	Log::Print(Log::EMsg, "New allocation Size %i Name \"%s\" File \"%s:%i\"", size, pName, file, line);
	return newStuff;
}
void* operator new[](size_t size, size_t alignment, size_t /* alignmentOffset */,
	const char* /* pName */, int /* flags */, unsigned /* debugFlags */, const char* /* file */, int /* line */)
{
	// this allocator doesn't support alignment
	//EASTL_ASSERT(alignment <= 8);
	return _aligned_malloc(size, alignment);
}