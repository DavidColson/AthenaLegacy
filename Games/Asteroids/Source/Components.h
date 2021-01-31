#pragma once

#include <TypeSystem.h>
#include <Vec2.h>
#include <Vec3.h>
#include <Log.h>
#include <Scene.h>
#include <AudioDevice.h>
#include <EASTL/fixed_vector.h>

#include "Asteroids.h"

struct CDynamics
{
	Vec3f vel;
	Vec3f accel;

	REFLECT()
};

struct CMenuInteraction
{
	enum State
	{
		Start,
		Quit
	};

	State currentState;
};

struct CShipDraw
{
	REFLECT();
};

struct CAsteroidSpawner
{
	float timer{ 30.0f };
	float timeBetweenSpawns{ 3.0f };
	float decay{ 0.98f };

	REFLECT();
};

struct CPlayerControl
{
	float thrust{ 160.f };
	float rotateSpeed{ 5.0f };
	float dampening{ 0.f };
	float respawnTimer{ 0.0f };
	int 	lives = 3;
	SoundID enginePlayingSound{ SoundID(-1) };

	// #RefactorNote: move this into the singleton HUD component
	EntityID lifeEntities[3]; // Stores the entityID of the three lives living in the corner of the screen
	REFLECT()
};

struct CGameOver
{
};

struct CPlayerScore
{
	int score;
};

struct CPlayerUI
{
	EntityID currentScoreEntity;
	EntityID gameOverEntity;
};

struct CSounds
{
	AssetHandle shootSound;
	AssetHandle explosionSound;
	AssetHandle engineSound;
};

struct CInvincibility
{
	float m_timer{ 5.0f };
	float m_flashTimer{ 0.3f };

	REFLECT()
};

struct CBullet
{
	float speed{ 700.0f };

	REFLECT()
};

struct CCollidable
{
	float radius{ 40.f };

	REFLECT()
};

struct CAsteroid
{
	int hitCount{ 0 };
	AsteroidType type;

	REFLECT()
};
