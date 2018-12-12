#include "Components/Components.h"

#include <GameFramework/World.h>
#include <Reflection.h>

REGISTRATION
{
	RegisterNewTypeAsComponent(CPlayerControl)
		->RegisterMember("m_thrust", &CPlayerControl::m_thrust)
		->RegisterMember("m_rotateSpeed", &CPlayerControl::m_rotateSpeed)
		->RegisterMember("m_dampening", &CPlayerControl::m_dampening)
		->RegisterMember("m_someVec", &CPlayerControl::m_someVec);

	RegisterNewTypeAsComponent(CDrawable)
		->RegisterMember("m_lineThickness", &CDrawable::m_lineThickness);

	RegisterNewTypeAsComponent(CTransform)
		->RegisterMember("m_pos", &CTransform::m_pos)
		->RegisterMember("m_rot", &CTransform::m_rot)
		->RegisterMember("m_sca", &CTransform::m_sca)
		->RegisterMember("m_vel", &CTransform::m_vel);

	RegisterNewTypeAsComponent(CBullet)
		->RegisterMember("m_speed", &CBullet::m_speed);

	RegisterNewTypeAsComponent(CCollidable)
		->RegisterMember("m_radius", &CCollidable::m_radius)
		->RegisterMember("m_colliding", &CCollidable::m_colliding)
		->RegisterMember("m_other", &CCollidable::m_other);
}
