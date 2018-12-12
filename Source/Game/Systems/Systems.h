#pragma once

#include <GameFramework/World.h>


void CollisionSystemUpdate(Scene* scene, float deltaTime);

void AsteroidSystemUpdate(Scene* scene, float deltaTime);

void DrawShapeSystem(Scene* scene, float deltaTime);

void MovementSystemUpdate(Scene* scene, float deltaTime);

void ShipControlSystemUpdate(Scene* scene, float deltaTime);
