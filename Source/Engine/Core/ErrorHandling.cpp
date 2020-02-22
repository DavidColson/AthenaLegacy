#include "ErrorHandling.h"

#include <string>
#include <memory>
#include <cstdlib>
#include <SDL.h>

#include "Log.h"

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

int ShowAssertDialog(const char * errorMsg, const char * file, int line)
{
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Abort" },
	{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Debug" },
	{ 0, 2, "Continue" },
	};

	std::string message = string_format("Assertion Failed\n\n%s\n\nFile: %s\nLine %i", errorMsg, file, line);

	Log::Print(Log::EErr, "%s", message.c_str());

	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_ERROR,
		NULL,
		"Error",
		message.c_str(),
		SDL_arraysize(buttons),
		buttons,
		nullptr
	};
	int buttonid;
	SDL_ShowMessageBox(&messageboxdata, &buttonid);
	return buttonid;
}

