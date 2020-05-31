
#include <cppfs/windows/LocalFileSystem.h>

#include <cppfs/FileHandle.h>
#include <cppfs/windows/LocalFileHandle.h>
#include <cppfs/windows/LocalFileWatcher.h>


namespace cppfs
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


} // namespace cppfs
