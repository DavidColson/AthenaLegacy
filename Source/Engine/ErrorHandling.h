#pragma once

#include <cstdlib>

#define ASSERT(condition, text) ((condition) ? (void)0 : Assertion(text, __FILE__, __LINE__))

int ShowAssertDialog(const char* errorMsg, const char* file, int line);

inline void Assertion(const char* errorMsg, const char* file, int line)
{
	switch (ShowAssertDialog(errorMsg, file, line))
	{
	case 0:
		_set_abort_behavior(0, _WRITE_ABORT_MSG);
		abort();
		break;
	case 1:
		__debugbreak();
		break;
	default:
		break;
	}
	return;
}
