#pragma once

#include <EASTL/string.h>

eastl::string DecodeBase64(eastl::string const& encoded_string);

eastl::string EncodeBase64(size_t length, const char* bytes);
