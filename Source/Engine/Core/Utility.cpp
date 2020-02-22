#include "Utility.h"

#include <stdio.h>
#include <stdarg.h>

std::string StringFormat(const char* text, ...)
{
	const int n = 1024;
	char buf[n];
	va_list args;
	va_start(args, text);
	vsnprintf(buf, n, text, args);
	buf[n - 1] = 0;
	va_end(args);

	return std::string(buf);
}
