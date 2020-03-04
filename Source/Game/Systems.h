#pragma once

#include <vector>
#include <Scene.h>
#include <Vec2.h>
#include <EASTL/fixed_vector.h>

// Utilities

eastl::fixed_vector<Vec2f, 15> GetRandomAsteroidMesh();

// Systems

void CollisionSystemUpdate(Scene& scene, float deltaTime);

void InvincibilitySystemUpdate(Scene& scene, float deltaTime);

void DrawPolyShapes(Scene& scene, float deltaTime);

void AsteroidSystemUpdate(Scene& scene, float deltaTime);

void MovementSystemUpdate(Scene& scene, float deltaTime);

void ShipControlSystemUpdate(Scene& scene, float deltaTime);

void MenuInterationSystem(Scene& scene, float deltaTime);

void OnPlayerControlRemoved(Scene& scene, EntityID entity);