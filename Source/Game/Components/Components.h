#pragma once

#include <TypeData.h>
#include <Maths/Maths.h>
#include <Maths/Vec2.h>
#include <Renderer/RenderProxy.h>
#include <Log.h>
#include <GameFramework/World.h>


struct CTransform
{
	vec3 m_pos;
	float m_rot;
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };
	vec3 m_vel{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 m_accel{ vec3(0.0f, 0.0f, 0.0f) };
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
	vec2 m_someVec{ vec2(5.0f, 2.0f) };
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
	bool m_colliding{ false };
	bool m_lastColliding{ false };
	bool m_collisionEnter{ false };
	bool m_collisionExit{ false };
	EntityID m_other;
};
REGISTER_EXTERN(CCollidable);

struct CAsteroid
{
	int m_hitCount{ 0 };
};
REGISTER_EXTERN(CAsteroid);
