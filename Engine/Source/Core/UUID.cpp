#include "UUID.h"

#include "ErrorHandling.h"

#ifdef _WIN32
#include <objbase.h>
#endif

Uuid::Uuid()
{
    for(int i=0; i<2; i++)
        data.u64[i] = 0;
}

Uuid Uuid::New()
{
#ifdef _WIN32
    Uuid uuid;
    CoCreateGuid((GUID*)&uuid);
    return uuid;
#else
    ASSERT(false, "UUIDs not working on this platform");
#endif
}

eastl::string Uuid::ToString()
{
    eastl::string output;
    output.sprintf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    data.u8[0],  data.u8[1], data.u8[2],data.u8[3],
    data.u8[4],  data.u8[5],
    data.u8[6],  data.u8[7],
    data.u8[8],  data.u8[9],
    data.u8[10], data.u8[11], data.u8[12], data.u8[13], data.u8[14], data.u8[15]);
    return output;
}

bool Uuid::IsNill()
{
    return data.u64[0] == 0 && data.u64[1] == 0;
}   

bool Uuid::operator==(const Uuid& other) const
{
    return data.u64[0] == other.data.u64[0] && data.u64[1] == other.data.u64[1];
}

bool Uuid::operator!=(const Uuid& other) const
{
    return data.u64[0] != other.data.u64[0] && data.u64[1] != other.data.u64[1];
}

bool Uuid::operator<(const Uuid& other) const
{
    return data.u64[0] < other.data.u64[0];
}