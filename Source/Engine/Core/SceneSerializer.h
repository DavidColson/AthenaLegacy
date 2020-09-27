#pragma once

#include "Scene.h"

#include <EASTL/string.h>

namespace SceneSerializer
{
    JsonValue ToJson(Scene& scene);
    Scene* NewSceneFromJson(JsonValue json);
}