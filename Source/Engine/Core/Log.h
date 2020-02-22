#pragma once

#include <vector>
#include <string>

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

	std::vector<std::string> GetLogHistory();
}