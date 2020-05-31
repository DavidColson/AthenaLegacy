
#pragma once


#include <memory>
#include <string>

#include <cppfs/AbstractFileSystem.h>


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
std::shared_ptr<AbstractFileSystem> localFS();

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
FileHandle open(const std::string & path);

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
std::string hashToString(const unsigned char * hash);


} // namespace fs


} // namespace cppfs
