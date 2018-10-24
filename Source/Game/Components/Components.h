
#include <Maths/Maths.h>
#include <Renderer/RenderProxy.h>

struct CTransform
{
	vec3 m_pos;
	float m_rot;
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };
};

struct CDrawable
{
	RenderProxy m_renderProxy;
};

struct CPlayerControl
{
	vec3 m_moveSpeed{ vec3(5.f, 5.f, 5.f) };
};