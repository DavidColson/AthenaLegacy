#include "Components/Components.h"

#include <GameFramework/World.h>
#include <Reflection.h>

REGISTRATION
{
	RegisterNewTypeAsComponent(CPlayerControl)
	->RegisterMember("m_thrust", &CPlayerControl::m_thrust)
	->RegisterMember("m_rotateSpeed", &CPlayerControl::m_rotateSpeed)
	->RegisterMember("m_dampening", &CPlayerControl::m_dampening)
	->RegisterMember("m_pos", &CPlayerControl::m_pos);

	RegisterNewTypeAsComponent(CDrawable);

	RegisterNewTypeAsComponent(CTransform);
}
