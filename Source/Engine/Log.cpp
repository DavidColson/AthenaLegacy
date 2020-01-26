#include "Log.h"

#include <Windows.h>
#include <string>
#include <stdio.h>

FILE* pFile{ nullptr };
std::vector<std::string> logHistory;

void Log::Print(LogType type, const char* text, ...)
{
	const int n = 1024;
	char buf[n];
	va_list args;
	va_start(args, text);
	vsnprintf(buf, n, text, args);
	buf[n - 1] = 0;
	va_end(args);

	if (pFile == nullptr)
		fopen_s(&pFile, "engine.log", "w");

	std::string prefix;
	switch (type)
	{
	case EMsg: prefix = "[MSG] "; break;
	case EWarn: prefix = "[WARN] "; break;
	case EErr: prefix = "[ERR] "; break;
	case EGraphics: prefix = "[GFXDEVICE] "; break;
	case EAudio: prefix = "[AUDIO] "; break;
	default: break;
	}

	std::string message = prefix;
	message += buf;
	message += "\n";

	fprintf(pFile, message.c_str());
	if (type != LogType::EGraphics)
		OutputDebugString(message.c_str());
	logHistory.push_back(message);
}

std::vector<std::string> Log::GetLogHistory()
{
	return logHistory;
}
