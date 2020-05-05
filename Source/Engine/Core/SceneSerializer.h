#pragma once

#include "Scene.h"

#include <EASTL/string.h>

namespace SceneSerializer
{
    eastl::string Serialize(Scene& scene);
}