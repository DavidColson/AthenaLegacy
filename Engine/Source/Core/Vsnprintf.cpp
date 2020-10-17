#include <stdio.h>

// A requirement of EASTL that we provide these functions for string::sprintf to work

int Vsnprintf8(char* p, size_t n, const char* pFormat, va_list arguments)
{
    return vsnprintf(p, n, pFormat, arguments);
}

int Vsnprintf16(char16_t* p, size_t n, const char16_t* pFormat, va_list arguments)
{
    return vswprintf_s((wchar_t*)p, n, (wchar_t*)pFormat, arguments);
}