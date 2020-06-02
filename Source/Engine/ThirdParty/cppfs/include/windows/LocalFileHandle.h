
#pragma once


#include <EASTL/memory.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>

#include <AbstractFileHandleBackend.h>


namespace FileSys
{


class LocalFileSystem;


/**
*  @brief
*    File handle for the local file system
*/
class LocalFileHandle : public AbstractFileHandleBackend
{
public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] fs
    *    File system that created this handle
    *  @param[in] path
    *    Path to file or directory
    */
    LocalFileHandle(eastl::shared_ptr<LocalFileSystem> fs, const eastl::string & path);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~LocalFileHandle();

    // Virtual AbstractFileHandleBackend functions
    virtual eastl::unique_ptr<AbstractFileHandleBackend> clone() const override;
    virtual AbstractFileSystem * fs() const override;
    virtual void updateFileInfo() override;
    virtual eastl::string path() const override;
    virtual bool exists() const override;
    virtual bool isFile() const override;
    virtual bool isDirectory() const override;
    virtual bool isSymbolicLink() const override;
    virtual eastl::vector<eastl::string> listFiles() const override;
    virtual eastl::unique_ptr<AbstractFileIteratorBackend> begin() const override;
    virtual unsigned int size() const override;
    virtual unsigned int accessTime() const override;
    virtual uint64_t modificationTime() const override;
    virtual unsigned int userId() const override;
    virtual void setUserId(unsigned int uid) override;
    virtual unsigned int groupId() const override;
    virtual void setGroupId(unsigned int gid) override;
    virtual unsigned long permissions() const override;
    virtual void setPermissions(unsigned long permissions) override;
    virtual bool createDirectory() override;
    virtual bool removeDirectory() override;
    virtual bool copy(AbstractFileHandleBackend & dest) override;
    virtual bool move(AbstractFileHandleBackend & dest) override;
    virtual bool createLink(AbstractFileHandleBackend & dest) override;
    virtual bool createSymbolicLink(AbstractFileHandleBackend & dest) override;
    virtual bool rename(const eastl::string & filename) override;
    virtual bool remove() override;
    virtual FileStream createFileStream(unsigned int mode) const override;


protected:
    void readFileInfo() const;


protected:
    eastl::shared_ptr<LocalFileSystem>   m_fs;       ///< File system that created this handle
    eastl::string                        m_path;     ///< Path to file or directory
    mutable void                        *m_fileInfo; ///< Information about the current file (created on demand)
};


} // namespace FileSys
