#include "Text.h"

#include <cppfs/FileSys.h>

void Text::Load(eastl::string path)
{
    FileSys::FileHandle fHandle = FileSys::open(path);
    contents = fHandle.readFile();
    AssetDB::RegisterAsset(this, path);
}