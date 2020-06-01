
#include <AbstractFileWatcherBackend.h>

#include <FileWatcher.h>


namespace FileSys
{


AbstractFileWatcherBackend::AbstractFileWatcherBackend(FileWatcher * fileWatcher)
: m_fileWatcher(fileWatcher)
{
}

AbstractFileWatcherBackend::~AbstractFileWatcherBackend()
{
}

void AbstractFileWatcherBackend::onFileEvent(FileHandle & fh, FileEvent event)
{
    m_fileWatcher->onFileEvent(fh, event);
}


} // namespace FileSys
