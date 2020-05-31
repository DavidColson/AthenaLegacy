
#pragma once


#include <string>


namespace FileSys
{


/**
*  @brief
*    Access system information
*/
namespace system
{


/**
*  @brief
*    Get home directory of the current user
*
*  @return
*    Home directory (native path)
*
*  @remarks
*    It is assumed that the home directory doesn't change
*    for the process lifetime.
*/
const std::string & homeDir();

/**
*  @brief
*    Get config directory for the named application
*
*  @param[in] application
*    Application name
*
*  @return
*    Config directory (native path)
*
*  @remarks
*    It is assumed that the config directory doesn't change
*    for the process lifetime.
*/
std::string configDir(const std::string & application);


} // namespace system


} // namespace FileSys
