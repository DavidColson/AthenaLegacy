#pragma once

#include <TypeSystem.h>
#include <Vec2.h>
#include <Vec3.h>
#include <Log.h>
#include <Scene.h>
#include <AudioDevice.h>

struct CPlayerControl
{
	float thrust{ 160.f };
	float rotateSpeed{ 0.1f };
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
	LoadedSoundHandle shootSound;
	LoadedSoundHandle explosionSound;
	LoadedSoundHandle engineSound;
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

	REFLECT()
};
