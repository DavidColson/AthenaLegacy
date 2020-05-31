
#include <cppfs/fs.h>

#include <sstream>
#include <iomanip>
#include <EASTL/iterator.h>

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


eastl::shared_ptr<AbstractFileSystem> localFS()
{
    static eastl::shared_ptr<LocalFileSystem> fs(new LocalFileSystem);

    return fs;
}

FileHandle open(const eastl::string & path)
{
    // Parse url
    Url url(path);

    // Get local path
    eastl::string localPath = url.path();

    // Open local file system
    auto fs = localFS();

    // Open path
    return fs->open(localPath);
}

eastl::string hashToString(const unsigned char * hash)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(2);

    for (int i=0; i<20; i++)
    {
        stream << static_cast<unsigned int>(hash[i]);
    }

    return stream.str().c_str();
}


} // namespace fs
} // namespace cppfs
