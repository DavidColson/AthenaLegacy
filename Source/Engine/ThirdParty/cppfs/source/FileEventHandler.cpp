
#include <FileEventHandler.h>

#include <FileHandle.h>


namespace FileSys
{


FileEventHandler::FileEventHandler()
{
}

FileEventHandler::~FileEventHandler()
{
}

void FileEventHandler::onFileEvent(FileHandle & fh, FileEvent event)
{
    switch (event) {
        case FileCreated:
            onFileCreated(fh);
            break;

        case FileRemoved:
            onFileRemoved(fh);
            break;

        case FileModified:
            onFileModified(fh);
            break;

        case FileAttrChanged:
            onFileAttrChanged(fh);
            break;

        default:
            break;
    }
}

void FileEventHandler::onFileCreated(FileHandle &)
{
}

void FileEventHandler::onFileRemoved(FileHandle &)
{
}

void FileEventHandler::onFileModified(FileHandle &)
{
}

void FileEventHandler::onFileAttrChanged(FileHandle &)
{
}


} // namespace FileSys
