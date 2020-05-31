#pragma once

#include <EASTL/string.h>

struct SDL_RWops;

namespace FileSys
{

enum SeekFrom
{
    SeekStart,
    SeekCurr,
    SeekEnd
};

class FileStream
{
public:
    /**
    *  @brief
    *    Constructor
    */
    FileStream();

     /**
    *  @brief
    *    Copy constructor
    *
    *  @param[in] fileStream
    *    Source stream
    */
    FileStream(const FileStream & fileStream);

    /**
    *  @brief
    *    Move constructor
    *
    *  @param[in] fileStream
    *    Source stream
    */
    FileStream(FileStream && fileStream);

    /**
    *  @brief
    *    Normal constructor
    *
    *  @param[in] path
    *    Path of file
    *  @param[in] mode
    *    Flags for mode of operation on filestream, See FileStreamMode enum for options
    */
    FileStream(eastl::string _path, unsigned int mode);

    /**
    *  @brief
    *    Destructor
    */
    ~FileStream();

    /**
    *  @brief
    *    Closes this file stream, invalid after this call
    */
    void close();

    /**
    *  @brief
    *    Reads from the stream into a string, taking length
    *
    *  @param[in] length
    *    How many bytes of the stream to read in from seek position
    * 
    *  @return
    *    What has been read, saved to a string
    */
    eastl::string read(size_t length);
    
    /**
    *  @brief
    *    Reads from the string into a provided buffer
    *
    *  @param[out] buffer
    *    Char buffer to write out what has been read
    *  @param[in] length
    *    How many bytes of the stream to read in from the seek position
    */
    void read(char* buffer, size_t length);

    /**
    *  @brief
    *    Writes buffer out to the stream
    *
    *  @param[in] buffer
    *    Char buffer to write out
    *  @param[in] length
    *    How many bytes of buffer to write out
    */
    void write(const char* buffer, size_t length);

    /**
    *  @brief
    *    Seeks the read/write cursor to a certain location
    *
    *  @param[in] offset
    *    Byte offset from the "from" location
    *  @param[in] from
    *    Where to seek from, current location, start or end?
    * 
    *  @return
    *    Where the cursor is after operation is complete
    */
    size_t seek(size_t offset, SeekFrom from = SeekCurr);

    /**
    *  @brief
    *    Gives back the current seek/cursor location
    *
    *  @return
    *    Location of the seek cursor
    */
    size_t tell();

    /**
    *  @brief
    *    Gives back the size of the stream in bytes
    *
    *  @return
    *    Byte size
    */
    size_t size();

    /**
    *  @brief
    *    Tells you if this stream is valid and can be used
    *
    *  @return
    *    true if valid
    */
    bool isValid();

private:
    SDL_RWops* rwops{ nullptr };
    eastl::string path{ "" };
    eastl::string modeString{ "" };
};

}