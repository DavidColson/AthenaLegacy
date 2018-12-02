#pragma once

#include <Maths/Maths.h>
#include <Renderer/RenderProxy.h>
#include <Log.h>

struct CTransform
{
	vec3 m_pos;
	float m_rot;
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };
	vec3 m_vel{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 m_accel{ vec3(0.0f, 0.0f, 0.0f) };
};

struct CDrawable
{
	RenderProxy m_renderProxy;
	float m_lineThickness{ 2.0f };
};

struct CPlayerControl
{
	float m_thrust{ 80.f };
	float m_rotateSpeed{ 0.1f };
	float m_dampening{ 0.f };
	vec2 m_someVec{ vec2(5.0f, 2.0f) };
};
