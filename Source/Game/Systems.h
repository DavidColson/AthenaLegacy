#pragma once

#include <vector>
#include <Scene.h>

void CollisionSystemUpdate(Scene& scene, float deltaTime);

void InvincibilitySystemUpdate(Scene& scene, float deltaTime);

void AsteroidSystemUpdate(Scene& scene, float deltaTime);

void MovementSystemUpdate(Scene& scene, float deltaTime);

void ShipControlSystemUpdate(Scene& scene, float deltaTime);
