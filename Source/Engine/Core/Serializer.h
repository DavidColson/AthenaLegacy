#pragma once

#include "EASTL/string.h"
#include "Variant.h"

namespace Serializer
{
    eastl::string Serialize(Variant value);
    Variant DeSerialize(eastl::string value);
}