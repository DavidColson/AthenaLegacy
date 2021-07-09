#pragma once

#include <TypeSystem.h>
#include <Vec2.h>
#include <Vec3.h>
#include <Log.h>
#include <Scene.h>
#include <AudioDevice.h>
#include <EASTL/fixed_vector.h>
#include <IComponent.h>
#include <SpatialComponent.h>

struct AsteroidComponent : public IComponent
{
	REFLECT_DERIVED();

	int hitCount{ 0 };
};

enum class CollisionType
{
	Player,
	Asteroid,
	Bullet
};

struct AsteroidPhysics : public SpatialComponent
{
	Vec3f velocity;
	Vec3f acceleration;
    float collisionRadius{ 1.0f };
	bool wrapAtEdge{ true };
	CollisionType type{ CollisionType::Asteroid };
	
	REFLECT_DERIVED()
};

struct Polyline : public SpatialComponent
{
	eastl::fixed_vector<Vec2f, 15> points;
	float thickness{ 5.0f };
	bool connected{ true };
	bool visible{ true };
	
	REFLECT_DERIVED()
};

struct PlayerComponent : public IComponent
{
    float thrust{ 160.f };
	float rotateSpeed{ 5.0f };
	float dampening{ 0.f };

	eastl::vector<Uuid> lives;
	Uuid playerPolylineComponent;
	
	bool hasCollidedWithAsteroid{ false };

	float respawnTimer{ 0.0f };
	float invicibilityTimer{ 5.0f };
	float flashTimer{ 0.3f };

	AssetHandle shootSound;
	AssetHandle explosionSound;
	AssetHandle engineSound;

	SoundID enginePlayingSound{ SoundID(-1) };

    REFLECT_DERIVED()
};

struct Score : public IComponent
{
	int currentScore{ 0 };
	int highScore{ 0 };

	bool update{ false };

	Uuid currentScoreTextElement;
	Uuid highScoreTextElement;
	Uuid gameOverTextElement;
	Uuid restartTextElement;

	bool gameOver{ false };

	REFLECT_DERIVED()
};

struct AsteroidSpawnData : public IComponent
{
	float timer = 30.0f;
	float timeBetweenSpawns = 3.0f;
	float decay = 0.98f;

	REFLECT_DERIVED()
};

struct MenuCursorComponent : public IComponent
{
	enum State
	{
		Start,
		Quit
	};

	State currentState;

	REFLECT_DERIVED()
};