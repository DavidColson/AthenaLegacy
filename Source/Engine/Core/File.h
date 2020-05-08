#pragma once

#include <EASTL/string.h>

namespace File
{
    eastl::string ReadWholeFile(eastl::string filepath, bool binary = false);

    void WriteWholeFile(eastl::string filepath, eastl::string contents, bool binary = false);
}