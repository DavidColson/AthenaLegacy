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
	float m_lineThickness{ 2.0f };
};
REGISTER_EXTERN(CDrawable);

struct CPlayerControl
{
	float m_thrust{ 80.f };
	float m_rotateSpeed{ 0.1f };
	float m_dampening{ 0.f };
	Vec2f m_newVec{ Vec2f(3.0f, 1.0f) };
};
REGISTER_EXTERN(CPlayerControl);

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
