#pragma once

#include <vector>
#include <Scene.h>
#include <Vec2.h>

// Utilities

std::vector<Vec2f> GetRandomAsteroidMesh();

// Systems

void CollisionSystemUpdate(Scene& scene, float deltaTime);

void InvincibilitySystemUpdate(Scene& scene, float deltaTime);

void DrawPolyShapes(Scene& scene, float deltaTime);

void AsteroidSystemUpdate(Scene& scene, float deltaTime);

void MovementSystemUpdate(Scene& scene, float deltaTime);

void ShipControlSystemUpdate(Scene& scene, float deltaTime);

void MenuInterationSystem(Scene& scene, float deltaTime);