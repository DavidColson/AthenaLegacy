#include "Text.h"

#include <FileSys.h>

void Text::Load(FileSys::FilePath path)
{
    FileSys::FileHandle fHandle = FileSys::open(path.fullPath());
    contents = fHandle.readFile();
}