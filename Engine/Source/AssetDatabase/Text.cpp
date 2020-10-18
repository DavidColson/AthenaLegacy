#include "Text.h"

#include <FileSystem.h>

void Text::Load(Path path, AssetHandle handleForThis)
{
    contents = FileSys::ReadWholeFile(path);
}