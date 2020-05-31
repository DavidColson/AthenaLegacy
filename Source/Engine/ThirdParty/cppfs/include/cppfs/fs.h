
#pragma once


#include <EASTL/memory.h>
#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

#include <cppfs/AbstractFileSystem.h>
#include <cppfs/FilePath.h>


namespace cppfs
{


class FileHandle;
class LoginCredentials;


/**
*  @brief
*    Global file system functions
*/
namespace fs
{


/**
*  @brief
*    Get local file system
*
*  @return
*    Pointer to local file system (never null)
*/
eastl::shared_ptr<AbstractFileSystem> localFS();

/**
*  @brief
*    Open a file or directory
*
*  @param[in] path
*    Path to file or directory
*  @param[in] credentials
*    Optional login credentials (can be null)
*
*  @return
*    File handle
*/
FileHandle open(const FilePath & path);

/**
*  @brief
*    Convert hash buffer into string
*
*  @param[in] hash
*    Hash buffer
*
*  @return
*    Hash string
*/
eastl::string hashToString(const unsigned char * hash);


} // namespace fs


} // namespace cppfs
