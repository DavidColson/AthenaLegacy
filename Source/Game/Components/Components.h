#pragma once

#include <TypeSystem.h>
#include <Maths/Vec2.h>
#include <Maths/Vec3.h>
#include <Renderer/RenderProxy.h>
#include <Log.h>
#include <GameFramework/World.h>


struct CTransform
{
	Vec3f m_pos;
	float m_rot;
	Vec3f m_sca{ Vec3f(1.f, 1.f, 1.f) };
	Vec3f m_vel{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f m_accel{ Vec3f(0.0f, 0.0f, 0.0f) };

	REFLECT()
};

struct CDrawable
{
	RenderProxy m_renderProxy;
	float m_lineThickness{ 1.5f };

	REFLECT()
};

struct CText
{
	std::string m_text;
	bool m_visible = true;

	REFLECT()
};

struct CPlayerControl
{
	float m_thrust{ 160.f };
	float m_rotateSpeed{ 0.1f };
	float m_dampening{ 0.f };
	float m_respawnTimer{ 0.0f };
	int 	m_lives = 3;
	// #RefactorNote: move this into the singleton HUD component
	EntityID m_lifeEntities[3]; // Stores the entityID of the three lives living in the corner of the screen
	// #RefactorNote Store an age timer, and give the player some invincibility when they spawn

	REFLECT()
};

struct CGameOver
{
};

struct CPlayerScore
{
	int m_score;
};

struct CBullet
{
	float m_speed = 300.0f;

	REFLECT()
};

struct CCollidable
{
	float m_radius{ 40.f };

	REFLECT()
};

struct CAsteroid
{
	int m_hitCount{ 0 };

	REFLECT()
};
