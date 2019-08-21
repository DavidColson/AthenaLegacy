#pragma once

#include <TypeData.h>
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
};
REGISTER_EXTERN(CTransform);

struct CDrawable
{
	RenderProxy m_renderProxy;
	float m_lineThickness{ 1.5f };
};
REGISTER_EXTERN(CDrawable);

struct CText
{
	std::string m_text;
	bool m_visible = true;
};
REGISTER_EXTERN(CText);

struct CPlayerControl
{
	float m_thrust{ 160.f };
	float m_rotateSpeed{ 0.1f };
	float m_dampening{ 0.f };
	float m_respawnTimer{ 0.0f };
	int m_lives{ 3 };
	// #RefactorNote Store an age timer, and give the player some invincibility when they spawn
};
REGISTER_EXTERN(CPlayerControl);

struct CGameOver
{
};

struct CPlayerScore
{
	int m_score;
};

struct CLife
{
	// #RefactorNote
	// This should probably store a reference to the player
	// And have a life UI system that checks the player and enables/disables
	// subentities attached to this to show the correct number of lives.
	// Maybe keep a pool of UI elements for each possible life shown up to some limit
};
REGISTER_EXTERN(CLife);

struct CBullet
{
	float m_speed = 300.0f;
};
REGISTER_EXTERN(CBullet);

struct CCollidable
{
	float m_radius{ 40.f };
};
REGISTER_EXTERN(CCollidable);

struct CAsteroid
{
	int m_hitCount{ 0 };
};
REGISTER_EXTERN(CAsteroid);
