
#pragma once


#include <EASTL/vector.h>
#include <iosfwd>

#include <Change.h>


namespace FileSys
{


/**
*  @brief
*    Representation of changes to a directory tree
*/
class Diff
{
public:
    /**
    *  @brief
    *    Constructor
    */
    Diff();

    /**
    *  @brief
    *    Destructor
    */
    ~Diff();

    /**
    *  @brief
    *    Clear data
    */
    void clear();

    /**
    *  @brief
    *    Get changes
    *
    *  @return
    *    List of changes
    */
    const eastl::vector<Change> & changes() const;

    /**
    *  @brief
    *    Add change
    *
    *  @param[in] change
    *    Change
    */
    void add(const Change & change);

    /**
    *  @brief
    *    Add change
    *
    *  @param[in] change
    *    Change
    */
    void add(Change && change);

    /**
    *  @brief
    *    Add change
    *
    *  @param[in] operation
    *    Operation type
    *  @param[in] path
    *    Path on which the operation takes place
    */
    void add(Change::Operation operation, const eastl::string & path);

    /**
    *  @brief
    *    Add change
    *
    *  @param[in] operation
    *    Operation type
    *  @param[in] path
    *    Path on which the operation takes place
    */
    void add(Change::Operation operation, eastl::string && path);

    /**
    *  @brief
    *    Print changes to string
    *
    */
    eastl::string toString();

    /**
    *  @brief
    *    Print changes to console
    */
    void logPrint();


protected:
    eastl::vector<Change> m_changes; ///< List of changes
};


} // namespace FileSys
