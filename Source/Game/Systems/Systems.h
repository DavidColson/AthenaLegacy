#pragma once

#include <GameFramework/World.h>


void CollisionSystemUpdate(Space* space, float deltaTime);

void AsteroidSystemUpdate(Space* space, float deltaTime);

void DrawShapeSystem(Space* space, float deltaTime);

void MovementSystemUpdate(Space* space, float deltaTime);

void ShipControlSystemUpdate(Space* space, float deltaTime);
