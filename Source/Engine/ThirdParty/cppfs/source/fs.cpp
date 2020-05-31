
#include <cppfs/fs.h>

#include <sstream>
#include <iomanip>
#include <iterator>

#if defined(__APPLE__)
    #define COMMON_DIGEST_FOR_OPENSSL
    #include <CommonCrypto/CommonDigest.h>
    #define SHA1 CC_SHA1
    #include <cppfs/ssh/SshFileSystem.h>
#elif defined(CPPFS_USE_OpenSSL)
    #include <openssl/sha.h>
    #include <cppfs/ssh/SshFileSystem.h>
#endif

#include <cppfs/system.h>
#include <cppfs/LoginCredentials.h>
#include <cppfs/Url.h>
#include <cppfs/FileHandle.h>
#include <cppfs/AbstractFileSystem.h>
#include <cppfs/FileIterator.h>

#ifdef SYSTEM_WINDOWS
    #include <cppfs/windows/LocalFileSystem.h>
#else
    #include <cppfs/posix/LocalFileSystem.h>
#endif


namespace cppfs
{
namespace fs
{


std::shared_ptr<AbstractFileSystem> localFS()
{
    static std::shared_ptr<LocalFileSystem> fs(new LocalFileSystem);

    return fs;
}

FileHandle open(const std::string & path)
{
    // Parse url
    Url url(path);

    // Get local path
    std::string localPath = url.path();

    // Open local file system
    auto fs = localFS();

    // Open path
    return fs->open(localPath);
}

std::string hashToString(const unsigned char * hash)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(2);

    for (int i=0; i<20; i++)
    {
        stream << static_cast<unsigned int>(hash[i]);
    }

    return stream.str();
}


} // namespace fs
} // namespace cppfs
