
#include <windows/LocalFileSystem.h>

#include <FileHandle.h>
#include <windows/LocalFileHandle.h>
#include <windows/LocalFileWatcher.h>


namespace FileSys
{


LocalFileSystem::LocalFileSystem()
{
}

LocalFileSystem::~LocalFileSystem()
{
}

FileHandle LocalFileSystem::open(const eastl::string & path)
{
    return open(eastl::string(path));
}

FileHandle LocalFileSystem::open(eastl::string && path)
{
    return FileHandle(
        eastl::unique_ptr<AbstractFileHandleBackend>(
            new LocalFileHandle(shared_from_this(), path)
        )
    );
}

eastl::unique_ptr<AbstractFileWatcherBackend> LocalFileSystem::createFileWatcher(FileWatcher & fileWatcher)
{
    return eastl::unique_ptr<AbstractFileWatcherBackend>(
            new LocalFileWatcher(&fileWatcher, shared_from_this())
    );
}


} // namespace FileSys
