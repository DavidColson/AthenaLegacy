#include "Base64.h"

#include "Log.h"

// ***********************************************************************

eastl::string printBits(int size, void const * const ptr)
{
    // Little endian
    eastl::string out;
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i = size - 1; i >= 0; i--)
    {
        for (j = 7; j >= 0; j--)
        {
            byte = (b[i] >> j) & 1;
            out.append_sprintf("%u", byte);
        }
    }
    return out;
}

static eastl::string lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

// ***********************************************************************

eastl::string DecodeBase64(eastl::string const& encodedString)
{
    if (encodedString.length() % 4 != 0)
    {
        Log::Crit("Invalid base64 encoded string");
        return "";
    }

    eastl::string output;
    output.reserve(encodedString.length());
    for( int i= 0; i < encodedString.length(); i += 4)
    {
        char a = lookup.find(encodedString[i]) & 0xFF;
        char b = lookup.find(encodedString[i + 1]) & 0xFF;
        char c = lookup.find(encodedString[i + 2]) & 0xFF; 
        char d = lookup.find(encodedString[i + 3]) & 0xFF; 

        output += a << 2 | (b & 0x30) >> 4;
        if (c != 0x40)
            output += b << 4 | (c & 0x3c) >> 2;
        if (d != 0x40)
            output += (c << 6 | d);
    }

    return output;
}

// ***********************************************************************

eastl::string EncodeBase64(size_t length, const char* bytes)
{
    eastl::string output;
    output.reserve(length * 2);
    for( int i= 0; i < length; i += 3)
    {
        int nChars = (int)length - i;
        char a = bytes[i];
        char b = nChars == 1 ? 0xF : bytes[i + 1];
        char c = nChars < 3 ? 0x3F : bytes[i + 2]; 
        
        output += lookup[a >> 2];
        output += lookup[(a & 0x3) << 4 | b >> 4];
        output += lookup[b == 0xF ? 0x40 : (b & 0xF) << 2 | c >> 6];
        output += lookup[c == 0x3F ? 0x40 : (c & 0x3F)];
    }

    return output;
}