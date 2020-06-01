
#pragma once


#include <EASTL/memory.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>

#include <AbstractFileIteratorBackend.h>


namespace FileSys
{


class LocalFileSystem;


/**
*  @brief
*    File iterator for the local file system
*/
class LocalFileIterator : public AbstractFileIteratorBackend
{
public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] fs
    *    File system that created this iterator
    *  @param[in] path
    *    Path to file or directory
    */
    LocalFileIterator(eastl::shared_ptr<LocalFileSystem> fs, const eastl::string & path);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~LocalFileIterator();

    // Virtual AbstractFileIteratorBackend functions
    virtual eastl::unique_ptr<AbstractFileIteratorBackend> clone() const override;
    virtual AbstractFileSystem * fs() const override;
    virtual bool valid() const override;
    virtual eastl::string path() const override;
    virtual int index() const override;
    virtual eastl::string name() const override;
    virtual void next() override;


protected:
    void readNextEntry();


protected:
    eastl::shared_ptr<LocalFileSystem>   m_fs;         ///< File system that created this iterator
    eastl::string                        m_path;       ///< Path to file or directory
    int                                m_index;      ///< Index of the current entry
	void                             * m_findHandle; ///< Search handle
	void                             * m_findData;   ///< Information about the current file
};


} // namespace FileSys
