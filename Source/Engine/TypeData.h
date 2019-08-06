#pragma once

#include "TypeDB.h"

#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Maths/Vec4.h"
#include "GameFramework/World.h"

// Base Types
REGISTER_EXTERN(float);

// Maths
REGISTER_EXTERN(Vec2f);
REGISTER_EXTERN(Vec2d);
REGISTER_EXTERN(Vec3f);
REGISTER_EXTERN(Vec3d);
REGISTER_EXTERN(Vec4f);
REGISTER_EXTERN(Vec4d);

// Entity System
REGISTER_EXTERN(EntityID);