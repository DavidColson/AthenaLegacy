#include "Log.h"

#include <Windows.h>

FILE* pFile{ nullptr };
Log::StringHistoryBuffer logHistory(100, eastl::allocator("Log History"));
Log::LogLevel globalLevel{ Log::EDebug };

namespace Log
{
	void PushLogMessage(LogLevel level, LogStringStorage& message)
	{
		if (level > globalLevel)
			return;

		// TODO: Use SDL File IO here
		if (pFile == nullptr)
			fopen_s(&pFile, "engine.log", "w");
		fprintf(pFile, message.c_str());
		fflush(pFile);

		OutputDebugString(message.c_str());

		if (!logHistory.validate())
			return;

		logHistory.push_back();
		LogEntry& entry = logHistory.back();

		entry.level = level;
		entry.message = message;
	}
}

void Log::SetLogLevel(LogLevel level)
{
	globalLevel = level;
}

void Log::Crit(const char* text, ...)
{
	LogStringStorage message = "[CRITICAL] ";
	va_list arguments;
	va_start(arguments, text);
	message.append_sprintf_va_list(text, arguments);
	va_end(arguments);
	message += "\n";

	PushLogMessage(LogLevel::ECrit, message);
}

void Log::Warn(const char* text, ...)
{
	LogStringStorage message = "[WARN] ";
	va_list arguments;
	va_start(arguments, text);
	message.append_sprintf_va_list(text, arguments);
	va_end(arguments);
	message += "\n";

	PushLogMessage(LogLevel::EWarn, message);
}

void Log::Info(const char* text, ...)
{
	LogStringStorage message = "[INFO] ";
	va_list arguments;
	va_start(arguments, text);
	message.append_sprintf_va_list(text, arguments);
	va_end(arguments);
	message += "\n";

	PushLogMessage(LogLevel::EInfo, message);
}

void Log::Debug(const char* text, ...)
{
	LogStringStorage message = "[DEBUG] ";
	va_list arguments;
	va_start(arguments, text);
	message.append_sprintf_va_list(text, arguments);
	va_end(arguments);
	message += "\n";

	PushLogMessage(LogLevel::EDebug, message);
}

const Log::StringHistoryBuffer& Log::GetLogHistory()
{
	return logHistory;
}
