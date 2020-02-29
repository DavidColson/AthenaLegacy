#include "Log.h"

#include <Windows.h>
#include <string>
#include <stdio.h>

FILE* pFile{ nullptr };
StringHistoryBuffer logHistory(100, eastl::allocator("Log History"));

void Log::Print(LogType type, const char* text, ...)
{
	if (!logHistory.validate())
		return;

	logHistory.push_back();

	const int n = 1024;
	char buf[n];
	va_list args;
	va_start(args, text);
	vsnprintf(buf, n, text, args);
	buf[n - 1] = 0;
	va_end(args);

	if (pFile == nullptr)
		fopen_s(&pFile, "engine.log", "w");

	Fixed1024String& message = logHistory.back();
	switch (type)
	{
	case EMsg: message = "[MSG] "; break;
	case EWarn: message = "[WARN] "; break;
	case EErr: message = "[ERR] "; break;
	case EGraphics: message = "[GFXDEVICE] "; break;
	case EAudio: message = "[AUDIO] "; break;
	default: break;
	}

	message += buf;
	message += "\n";

	fprintf(pFile, message.c_str());
	if (type != LogType::EGraphics)
		OutputDebugString(message.c_str());
}

const StringHistoryBuffer& Log::GetLogHistory()
{
	return logHistory;
}
