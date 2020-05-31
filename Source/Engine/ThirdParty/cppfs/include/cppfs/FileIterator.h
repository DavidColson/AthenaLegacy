
#pragma once


#include <EASTL/memory.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>

namespace cppfs
{


class AbstractFileSystem;
class AbstractFileIteratorBackend;


/**
*  @brief
*    File iterator
*/
class FileIterator
{
public:
    /**
    *  @brief
    *    Constructor
    */
    FileIterator();

    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] backend
    *    Concrete file iterator backend
    */
    FileIterator(eastl::unique_ptr<AbstractFileIteratorBackend> && backend);

    /**
    *  @brief
    *    Copy constructor
    *
    *  @param[in] fileIterator
    *    Source iterator
    */
    FileIterator(const FileIterator & fileIterator);

    /**
    *  @brief
    *    Move constructor
    *
    *  @param[in] fileIterator
    *    Source iterator
    */
    FileIterator(FileIterator && fileIterator);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~FileIterator();

    /**
    *  @brief
    *    Copy operator
    *
    *  @param[in] fileIterator
    *    Source iterator
    */
    FileIterator & operator=(const FileIterator & fileIterator);

    /**
    *  @brief
    *    Get name of current directory item
    *
    *  @return
    *    File name
    */
    eastl::string operator*() const;

    /**
    *  @brief
    *    Advance to the next item
    */
    void operator++();

    /**
    *  @brief
    *    Compare two iterators
    *
    *  @param[in] it
    *    Iterator
    *
    *  @return
    *    'true' if iterators are equal, else 'false'
    */
    bool operator==(const FileIterator & it) const;

    /**
    *  @brief
    *    Compare two iterators
    *
    *  @param[in] it
    *    Iterator
    *
    *  @return
    *    'true' if iterators are not equal, else 'false'
    */
    bool operator!=(const FileIterator & it) const;

    /**
    *  @brief
    *    Get file system
    *
    *  @return
    *    File system (can be null)
    */
    AbstractFileSystem * fs() const;


protected:
    eastl::unique_ptr<AbstractFileIteratorBackend> m_backend;
};


} // namespace cppfs
