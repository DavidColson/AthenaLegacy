#include "Text.h"

#include <FileSystem.h>

void Text::Load(Path path)
{
    contents = FileSys::ReadWholeFile(path);
}