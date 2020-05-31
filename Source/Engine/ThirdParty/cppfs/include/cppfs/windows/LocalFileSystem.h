
#pragma once


#include <EASTL/memory.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>

#include <cppfs/AbstractFileSystem.h>


namespace cppfs
{


/**
*  @brief
*    Representation of the local file system
*/
class LocalFileSystem : public AbstractFileSystem, public eastl::enable_shared_from_this<LocalFileSystem>
{
public:
    /**
    *  @brief
    *    Constructor
    */
    LocalFileSystem();

    /**
    *  @brief
    *    Destructor
    */
    virtual ~LocalFileSystem();

    // Virtual AbstractFileSystem functions
    virtual FileHandle open(const eastl::string & path) override;
    virtual FileHandle open(eastl::string && path) override;
    virtual eastl::unique_ptr<AbstractFileWatcherBackend> createFileWatcher(FileWatcher & fileWatcher) override;
};


} // namespace cppfs
