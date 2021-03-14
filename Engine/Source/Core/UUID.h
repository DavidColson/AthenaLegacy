#pragma once

#include "EASTL/string.h"

class Uuid
{
public:
    Uuid();

    static Uuid New();

    eastl::string ToString();

    bool operator==(const Uuid& other);

    bool operator!=(const Uuid& other);

private:
    union{
        uint8_t u8[16];
        uint64_t u64[2];
    } data;
};