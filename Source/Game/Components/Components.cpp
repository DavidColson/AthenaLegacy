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

REGISTER(CLife)
{
	NewType(CLife);
}

REGISTER(CDrawable)
{
	NewType(CDrawable)
		->RegisterMember("m_lineThickness", &CDrawable::m_lineThickness);
}

REGISTER(CText)
{
	NewType(CText)
		->RegisterMember("m_text", &CText::m_text)
		->RegisterMember("m_visible", &CText::m_visible);
}

REGISTER(CTransform)
{
	NewType(CTransform)
		->RegisterMember("m_pos", &CTransform::m_pos)
		->RegisterMember("m_rot", &CTransform::m_rot)
		->RegisterMember("m_sca", &CTransform::m_sca)
		->RegisterMember("m_vel", &CTransform::m_vel);
}

REGISTER(CBullet)
{
	NewType(CBullet)
		->RegisterMember("m_speed", &CBullet::m_speed);
}

REGISTER(CAsteroid)
{
	NewType(CAsteroid)
		->RegisterMember("m_hitCount", &CAsteroid::m_hitCount);
}

REGISTER(CCollidable)
{
	NewType(CCollidable)
		->RegisterMember("m_radius", &CCollidable::m_radius);
}
