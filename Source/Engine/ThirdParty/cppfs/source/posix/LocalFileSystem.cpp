
#include <posix/LocalFileSystem.h>

#include <FileHandle.h>
#include <FileWatcher.h>
#include <AbstractFileWatcherBackend.h>
#include <posix/LocalFileHandle.h>

#ifdef SYSTEM_LINUX
    #include <linux/LocalFileWatcher.h>
#endif


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
#ifdef SYSTEM_LINUX
    return eastl::unique_ptr<AbstractFileWatcherBackend>(
            new LocalFileWatcher(&fileWatcher, shared_from_this())
    );
#else
    return nullptr;
#endif
}


} // namespace FileSys
