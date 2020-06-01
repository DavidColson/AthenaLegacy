
#include <FileVisitor.h>

#include <FileHandle.h>


namespace FileSys
{


FileVisitor::FileVisitor()
{
}

FileVisitor::~FileVisitor()
{
}

bool FileVisitor::onFileEntry(FileHandle & fh)
{
    if (fh.isDirectory())
    {
        return onDirectory(fh);
    }

    else
    {
        return onFile(fh);
    }
}

bool FileVisitor::onFile(FileHandle &)
{
    return true;
}

bool FileVisitor::onDirectory(FileHandle &)
{
    return true;
}


} // namespace FileSys
