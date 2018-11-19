
#include <Maths/Maths.h>
#include <Renderer/RenderProxy.h>
#include <Log.h>
#include <Reflection.h>

struct CTransform
{
	vec3 m_pos;
	float m_rot;
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };

	vec3 m_vel{ vec3(0.0f, 0.0f, 0.0f) };
};

struct CDrawable
{
	RenderProxy m_renderProxy;
};

struct CPlayerControl
{
	REFLECTABLE(CPlayerControl) // idea for initialization. Instead of relying on before main statics. Have a static GetType function in here
		// This will be a singleton creator for the Type object, creating or retrieving it. That way things are created in the order they're needed

	float m_thrust{ 80.f };
	float m_rotateSpeed{ 0.1f };
	float m_dampening{ 2.f };
	vec2 m_pos{ vec2(5.0f, 2.0f) };
};

REGISTRATION
{
	TypeDatabase::RegisterNewType<CPlayerControl>("CPlayerControl")
		->RegisterMember("m_thrust", &CPlayerControl::m_thrust)
		->RegisterMember("m_rotateSpeed", &CPlayerControl::m_rotateSpeed)
		->RegisterMember("m_dampening", &CPlayerControl::m_dampening)
		->RegisterMember("m_pos", &CPlayerControl::m_pos);
}
