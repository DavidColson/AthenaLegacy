
#pragma once

#include <FileStream.h>

#include <EASTL/memory.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>
#include <ios>
#include <iosfwd>


namespace FileSys
{


class AbstractFileSystem;
class AbstractFileIteratorBackend;


/**
*  @brief
*    Interface for file handles
*/
class AbstractFileHandleBackend
{
public:
    /**
    *  @brief
    *    Constructor
    */
    AbstractFileHandleBackend();

    /**
    *  @brief
    *    Destructor
    */
    virtual ~AbstractFileHandleBackend();

    /**
    *  @brief
    *    Create a copy of this handle
    *
    *  @return
    *    File handle
    */
    virtual eastl::unique_ptr<AbstractFileHandleBackend> clone() const = 0;

    /**
    *  @brief
    *    Get file system
    *
    *  @return
    *    File system (must NOT be null)
    */
    virtual AbstractFileSystem * fs() const = 0;

    /**
    *  @brief
    *    Update file information
    */
    virtual void updateFileInfo() = 0;

    /**
    *  @brief
    *    Get path
    *
    *  @return
    *    Path to file or directory
    */
    virtual eastl::string path() const = 0;

    /**
    *  @brief
    *    Check if file or directory exists
    *
    *  @return
    *    'true' if it exists, else 'false'
    */
    virtual bool exists() const = 0;

    /**
    *  @brief
    *    Check if item is a file
    *
    *  @return
    *    'true' if it is a file, else 'false'
    */
    virtual bool isFile() const = 0;

    /**
    *  @brief
    *    Check if item is a directory
    *
    *  @return
    *    'true' if it is a directory, else 'false'
    */
    virtual bool isDirectory() const = 0;

    /**
    *  @brief
    *    Check if item is a symbolic link
    *
    *  @return
    *    'true' if it is a symbolic link, else 'false'
    */
    virtual bool isSymbolicLink() const = 0;

    /**
    *  @brief
    *    List files in directory
    *
    *  @return
    *    List of files, empty list if this is not a valid directory
    */
    virtual eastl::vector<eastl::string> listFiles() const = 0;

    /**
    *  @brief
    *    Get iterator that points to the first directory entry
    *
    *  @return
    *    Iterator backend, must NOT be null
    */
    virtual eastl::unique_ptr<AbstractFileIteratorBackend> begin() const = 0;

    /**
    *  @brief
    *    Get file size
    *
    *  @return
    *    Size if handle points to a file, else 0
    */
    virtual unsigned int size() const = 0;

    /**
    *  @brief
    *    Get time of last access
    *
    *  @return
    *    Time stamp
    */
    virtual unsigned int accessTime() const = 0;

    /**
    *  @brief
    *    Get time of last modification
    *
    *  @return
    *    Time stamp
    */
    virtual uint64_t modificationTime() const = 0;

    /**
    *  @brief
    *    Get ID of owning user
    *
    *  @return
    *    User ID
    */
    virtual unsigned int userId() const = 0;

    /**
    *  @brief
    *    Set owning user
    *
    *  @param[in] uid
    *    User ID
    */
    virtual void setUserId(unsigned int uid) = 0;

    /**
    *  @brief
    *    Get ID of owning group
    *
    *  @return
    *    Group ID
    */
    virtual unsigned int groupId() const = 0;

    /**
    *  @brief
    *    Set owning group
    *
    *  @param[in] gid
    *    Group ID
    */
    virtual void setGroupId(unsigned int gid) = 0;

    /**
    *  @brief
    *    Get file permissions
    *
    *  @return
    *    File permissions
    */
    virtual unsigned long permissions() const = 0;

    /**
    *  @brief
    *    Set file permissions
    *
    *  @param[in] permissions
    *    File permissions
    */
    virtual void setPermissions(unsigned long permissions) = 0;

    /**
    *  @brief
    *    Create directory
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool createDirectory() = 0;

    /**
    *  @brief
    *    Remove directory
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool removeDirectory() = 0;

    /**
    *  @brief
    *    Copy file
    *
    *  @param[in] dest
    *    Destination file or directory (must be of the same type as this file handle)
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool copy(AbstractFileHandleBackend & dest) = 0;

    /**
    *  @brief
    *    Move file
    *
    *  @param[in] dest
    *    Destination file or directory (must be of the same type as this file handle)
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool move(AbstractFileHandleBackend & dest) = 0;

    /**
    *  @brief
    *    Create hard link
    *
    *  @param[in] dest
    *    Destination file or directory (must be of the same type as this file handle)
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool createLink(AbstractFileHandleBackend & dest) = 0;

    /**
    *  @brief
    *    Create symbolic link
    *
    *  @param[in] dest
    *    Destination file or directory (must be of the same type as this file handle)
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool createSymbolicLink(AbstractFileHandleBackend & dest) = 0;

    /**
    *  @brief
    *    Rename file or directory
    *
    *  @param[in] filename
    *    File name
    *
    *  @return
    *    'true' if successful, else 'false'
    */
    virtual bool rename(const eastl::string & filename) = 0;

    /**
    *  @brief
    *    Remove file
    *
    *  @return
    *    'true' if successful, else 'false'
    *
    *  @remarks
    *    Does only work if the handle points to a valid file,
    *    not a directory.
    */
    virtual bool remove() = 0;

    /**
    *  @brief
    *    Create file stream to read/write from the file
    *
    *  @param[in] mode
    *    Opening mode flags
    *
    *  @return
    *    File stream, invalid on error
    *
    *  @remarks
    *    The created stream object has to be destroyed be the caller.
    */
    virtual FileStream createFileStream(unsigned int mode) const = 0;
};


} // namespace FileSys
