#pragma once

namespace Log
{
	enum LogType
	{
		EMsg,
		EWarn,
		EErr
	};

	void Print(LogType type, const char* text, ...);
}