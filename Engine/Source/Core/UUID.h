#pragma once

#include "EASTL/string.h"

class Uuid
{
public:
    Uuid();

    static Uuid New();

    eastl::string ToString();

    bool IsNill();

    bool operator==(const Uuid& other) const;

    bool operator!=(const Uuid& other) const;

    bool operator<(const Uuid& other) const;

private:
    union{
        uint8_t u8[16];
        uint64_t u64[2];
    } data;
};