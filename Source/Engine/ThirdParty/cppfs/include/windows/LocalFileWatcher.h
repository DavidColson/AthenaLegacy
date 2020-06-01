
#pragma once


#include <mutex>
#include <EASTL/memory.h>
#include <EASTL/vector.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <AbstractFileWatcherBackend.h>
#include <FileHandle.h>


namespace FileSys
{


class LocalFileSystem;


/**
*  @brief
*    File watcher for the local file system
*/
class LocalFileWatcher : public AbstractFileWatcherBackend
{
public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] fileWatcher
    *    File watcher that owns the backend (must NOT be null!)
    *  @param[in] fs
    *    File system that created this watcher
    */
    LocalFileWatcher(FileWatcher * fileWatcher, eastl::shared_ptr<LocalFileSystem> fs);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~LocalFileWatcher();

    // Virtual AbstractFileWatcherBackend functions
    virtual AbstractFileSystem * fs() const override;
    virtual void add(FileHandle & dir, unsigned int events, RecursiveMode recursive) override;
    virtual void watch(int timeout) override;


protected:
    /**
    *  @brief
    *    Watcher entry
    */
    struct Watcher {
        FileHandle              dir;           ///< Directory that is watched
        unsigned int            events;        ///< Watched events
        RecursiveMode           recursive;     ///< Watch recursively?
        eastl::shared_ptr<void> dirHandle;     ///< Handle for the directory
        eastl::shared_ptr<void> event;         ///< Event that is triggered for this watcher
        ::OVERLAPPED            overlapped;    ///< Overlapped data (for asynchronous operation)
        char                    buffer[16384]; ///< Buffer for overlapped data (1024 * sizeof(FILE_NOTIFY_INFORMATION))
    };


protected:
    eastl::shared_ptr<LocalFileSystem> m_fs;            ///< File system that created this watcher
    eastl::vector<Watcher>             m_watchers;      ///< List of watchers
    ::HANDLE                           m_waitStopEvent; ///< Event to stop watch function
    ::CRITICAL_SECTION                 m_mutexWatchers; ///< Mutex/critical section for accessing m_watchers
};


} // namespace FileSys
