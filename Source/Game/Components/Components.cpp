#include "Components/Components.h"

#include <GameFramework/World.h>
#include <TypeDB.h>

REGISTER(CPlayerControl)
{
	NewType(CPlayerControl)
		->RegisterMember("m_thrust", &CPlayerControl::m_thrust)
		->RegisterMember("m_rotateSpeed", &CPlayerControl::m_rotateSpeed)
		->RegisterMember("m_dampening", &CPlayerControl::m_dampening);
}

REFLECT_BEGIN(CPlayerControl)
REFLECT_MEMBER(m_thrust)
REFLECT_MEMBER(m_rotateSpeed)
REFLECT_MEMBER(m_dampening)
REFLECT_END()

REGISTER(CDrawable)
{
	NewType(CDrawable)
		->RegisterMember("m_lineThickness", &CDrawable::m_lineThickness);
}

REFLECT_BEGIN(CDrawable)
REFLECT_MEMBER(m_lineThickness)
REFLECT_END()

REGISTER(CText)
{
	NewType(CText)
		->RegisterMember("m_text", &CText::m_text)
		->RegisterMember("m_visible", &CText::m_visible);
}

REFLECT_BEGIN(CText)
REFLECT_MEMBER(m_text)
REFLECT_MEMBER(m_visible)
REFLECT_END()

REGISTER(CTransform)
{
	NewType(CTransform)
		->RegisterMember("m_pos", &CTransform::m_pos)
		->RegisterMember("m_rot", &CTransform::m_rot)
		->RegisterMember("m_sca", &CTransform::m_sca)
		->RegisterMember("m_vel", &CTransform::m_vel);
}

REFLECT_BEGIN(CTransform)
REFLECT_MEMBER(m_pos)
REFLECT_MEMBER(m_rot)
REFLECT_MEMBER(m_sca)
REFLECT_MEMBER(m_vel)
REFLECT_END()

REGISTER(CBullet)
{
	NewType(CBullet)
		->RegisterMember("m_speed", &CBullet::m_speed);
}

REFLECT_BEGIN(CBullet)
REFLECT_MEMBER(m_speed)
REFLECT_END()

REGISTER(CAsteroid)
{
	NewType(CAsteroid)
		->RegisterMember("m_hitCount", &CAsteroid::m_hitCount);
}

REFLECT_BEGIN(CAsteroid)
REFLECT_MEMBER(m_hitCount)
REFLECT_END()

REGISTER(CCollidable)
{
	NewType(CCollidable)
		->RegisterMember("m_radius", &CCollidable::m_radius);
}

REFLECT_BEGIN(CCollidable)
REFLECT_MEMBER(m_radius)
REFLECT_END()
