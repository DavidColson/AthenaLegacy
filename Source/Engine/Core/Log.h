#pragma once

#include <EASTL/vector.h>
#include <EASTL/bonus/ring_buffer.h>
#include <EASTL/fixed_string.h>
#include <string>

typedef eastl::fixed_string<char, 1024, false> Fixed1024String;
typedef eastl::ring_buffer<Fixed1024String, eastl::vector<Fixed1024String>> StringHistoryBuffer;

namespace Log
{
	enum LogType
	{
		EMsg,
		EWarn,
		EErr,
    	EGraphics,
    	EAudio,
	};

	void Print(LogType type, const char* text, ...);

	const StringHistoryBuffer&  GetLogHistory();
}