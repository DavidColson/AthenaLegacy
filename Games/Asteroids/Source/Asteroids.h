#pragma once

#include <Scene.h>
#include "Rendering/GfxDraw.h"

#define PLAYER_ID EntityID{ 0 }

void LoadMainScene();
void LoadMenu();

enum class AsteroidType
{
    Type1,
    Type2,
    Type3,
    Type4
};

GfxDraw::PolyshapeMesh& GetAsteroidMesh(AsteroidType type);

GfxDraw::PolyshapeMesh& GetPlayerMesh();