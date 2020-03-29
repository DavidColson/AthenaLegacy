#pragma once

#include <EASTL/vector.h>
#include <EASTL/bonus/ring_buffer.h>
#include <EASTL/fixed_string.h>
#include <string>

namespace Log
{
	typedef eastl::fixed_string<char, 1024> LogStringStorage;
	
	enum LogLevel
	{
		ECrit,
		EWarn,
		EInfo,
		EDebug
	};

	struct LogEntry
	{
		LogLevel level;
		LogStringStorage message;
	};

	typedef eastl::ring_buffer<LogEntry, eastl::vector<LogEntry>> StringHistoryBuffer;

	void SetLogLevel(LogLevel level);
	void Crit(const char* text, ...);
	void Warn(const char* text, ...);
	void Info(const char* text, ...);
	void Debug(const char* text, ...);

	const StringHistoryBuffer&  GetLogHistory();
}
