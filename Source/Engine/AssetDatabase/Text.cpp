#include "Text.h"

#include "File.h"

void Text::Load(eastl::string path)
{
    contents = File::ReadWholeFile(path);
    AssetDB::RegisterAsset(this, path);
}