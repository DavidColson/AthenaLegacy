
#include <FileSys.h>

#include <sstream>
#include <iomanip>
#include <EASTL/iterator.h>

#if defined(__APPLE__)
    #define COMMON_DIGEST_FOR_OPENSSL
    #include <CommonCrypto/CommonDigest.h>
    #define SHA1 CC_SHA1
    #include <ssh/SshFileSystem.h>
#elif defined(CPPFS_USE_OpenSSL)
    #include <openssl/sha.h>
    #include <ssh/SshFileSystem.h>
#endif

#include <system.h>
#include <FileHandle.h>
#include <AbstractFileSystem.h>
#include <FileIterator.h>

#ifdef SYSTEM_WINDOWS
    #include <windows/LocalFileSystem.h>
#else
    #include <posix/LocalFileSystem.h>
#endif


namespace FileSys
{

eastl::shared_ptr<AbstractFileSystem> localFS()
{
    static eastl::shared_ptr<LocalFileSystem> fs(new LocalFileSystem);

    return fs;
}

FileHandle open(const FilePath & path)
{
    // Open local file system
    auto fs = localFS();

    // Open path
    return fs->open(path.fullPath());
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

} // namespace FileSys
