#pragma once

#include <vector>
#include <Renderer/RenderProxy.h>
#include <Scene.h>

void CollisionSystemUpdate(Scene& scene, float deltaTime);

void InvincibilitySystemUpdate(Scene& scene, float deltaTime);

void AsteroidSystemUpdate(Scene& scene, float deltaTime);

void DrawShapeSystem(Scene& scene, float deltaTime);

void DrawTextSystem(Scene& scene, float deltaTime);

void MovementSystemUpdate(Scene& scene, float deltaTime);

void ShipControlSystemUpdate(Scene& scene, float deltaTime);