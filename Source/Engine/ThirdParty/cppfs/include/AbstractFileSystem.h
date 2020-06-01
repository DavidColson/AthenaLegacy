
#pragma once


#include <EASTL/memory.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>

namespace FileSys
{


class FileHandle;
class FileWatcher;
class AbstractFileWatcherBackend;


/**
*  @brief
*    Interface for accessing file systems
*/
class AbstractFileSystem
{
public:
    /**
    *  @brief
    *    Constructor
    */
    AbstractFileSystem();

    /**
    *  @brief
    *    Destructor
    */
    virtual ~AbstractFileSystem();

    /**
    *  @brief
    *    Open file or directory in file system
    *
    *  @param[in] path
    *    Path to file or directory
    */
    virtual FileHandle open(const eastl::string & path) = 0;

    /**
    *  @brief
    *    Open file or directory in file system
    *
    *  @param[in] path
    *    Path to file or directory
    */
    virtual FileHandle open(eastl::string && path) = 0;

    /**
    *  @brief
    *    Create a watcher for the file system
    *
    *  @param[in] fileWatcher
    *    File watcher that owns the backend
    *
    *  @return
    *    Watcher backend (must NOT be null!)
    */
    virtual eastl::unique_ptr<AbstractFileWatcherBackend> createFileWatcher(FileWatcher & fileWatcher) = 0;
};


} // namespace FileSys
