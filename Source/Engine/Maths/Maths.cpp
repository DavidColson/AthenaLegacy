#include "Maths.h"

// casting constructors
template <> template <> vec<3, int>  ::vec(const vec<3, float> &v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> vec<3, float>::vec(const vec<3, int> &v) : x(float(v.x)), y(float(v.y)), z(float(v.z)) {}
template <> template <> vec<2, int>  ::vec(const vec<2, float> &v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> vec<2, float>::vec(const vec<2, int> &v) : x(float(v.x)), y(float(v.y)) {}

mat4 MakeScale(vec3 scaling)
{
	mat4 ret = mat4::identity();
	ret[0] = vec4(scaling.x, 0.f, 0.f, 0.f);
	ret[1] = vec4(0.f, scaling.y, 0.f, 0.f);
	ret[2] = vec4(0.f, 0.f, scaling.z, 0.f);
	ret[3] = vec4(0.f, 0.f, 0.f, 1.f);
	return ret;
}

mat4 MakeTranslate(vec3 translate)
{
	mat4 ret = mat4::identity();
	ret[0] = vec4(1.f, 0.f, 0.f, translate.x);
	ret[1] = vec4(0.f, 1.f, 0.f, translate.y);
	ret[2] = vec4(0.f, 0.f, 1.f, translate.z);
	ret[3] = vec4(0.f, 0.f, 0.f, 1.f);
	return ret;
}

mat4 MakeRotate(vec3 rotation)
{
	float x = rotation.x;
	float y = rotation.y;
	float z = rotation.z;

	mat4 rx = mat4::identity();
	rx[0] = vec4(1.f, 0.f,		0.f,		0.f);
	rx[1] = vec4(0.f, cosf(x),	-sinf(x),	0.f);
	rx[2] = vec4(0.f, sinf(x),	cosf(x),	0.f);
	rx[3] = vec4(0.f, 0.f,		0.f,		1.f);
	

	mat4 ry = mat4::identity();
	ry[0] = vec4(cosf(y),	0.f, -sinf(y),	0.f);
	ry[1] = vec4(0.f,		1.f, 0.f,		0.f);
	ry[2] = vec4(sinf(y),	0.f, cosf(y),	0.f);
	ry[3] = vec4(0.f,		0.f, 0.f,		1.f);

	mat4 rz = mat4::identity();
	rz[0] = vec4(cosf(z),	-sinf(z),	0.f, 0.f);
	rz[1] = vec4(sinf(z),	cosf(z),	0.f, 0.f);
	rz[2] = vec4(0.f,		0.f,		1.f, 0.f);
	rz[3] = vec4(0.f,		0.f,		0.f, 1.f);

	return rz * ry * rx;
}

mat4 MakePerspective(float ScreenWidth, float ScreenHeight, float Near, float Far, float FOV)
{
	float ar = ScreenWidth / ScreenHeight;
	float zRange = Near - Far;
	float tanHalfFOV = tanf(ToRadian(FOV * 0.5f));

	mat4 proj = mat4::identity();
	proj[0] = vec4((1.0f / (tanHalfFOV * ar)),	0.f,					0.f,					0.f);
	proj[1] = vec4(0.f,							(1.0f / (tanHalfFOV)),	0.f,					0.f);
	proj[2] = vec4(0.f,							0.f,					(-Near - Far) / zRange, (2 * Far * Near) / zRange);
	proj[3] = vec4(0.f,							0.f,					1.f,					0.f);

	return proj;
}

mat4 MakeOrthographic(float left, float right, float bottom, float top, float near, float far)
{
	mat4 ortho = mat4::identity();
	ortho[0] = vec4(2.0f / (right - left), 0.f, 0.f, (- right - left) / (right - left));
	ortho[1] = vec4(0.f, 2.0f / (top - bottom), 0.f, (- top - bottom) / (top - bottom));
	ortho[2] = vec4(0.f, 0.f, 1.0f / (far - near), (- near) / (far - near));
	ortho[3] = vec4(0.f, 0.f, 0.f, 1.f);

	return ortho;
}

mat4 MakeViewport(float width, float height)
{
	mat4 view = mat4::identity();
	view[0] = vec4(width / 2.f,		0.f,			0.f,			width / 2.f);
	view[1] = vec4(0.f,				height / 2.f,	0.f,			height / 2.f);
	view[2] = vec4(0.f,				0.f,			255.0 / 2.0,	255.0 / 2.0);
	view[3] = vec4(0.f,				0.f,			0.f,			1.f);

	return view;
}

mat4 MakeLookAt(vec3 Forward, vec3 Up)
{
	vec3 N = Forward;
	N .normalise();
	vec3 U = Up;
	U.normalise();
	U = cross(U, N);
	vec3 V = cross(N, U);

	mat4 lookat = mat4::identity();
	lookat[0] = vec4(U.x, U.y, U.z, 0.f);
	lookat[1] = vec4(V.x, V.y, V.z, 0.f);
	lookat[2] = vec4(N.x, N.y, N.z, 0.f);
	lookat[3] = vec4(0.f, 0.f, 0.f, 1.f);
	return lookat;
}

void GetAxesFromRotation(vec3 Rotation, vec3 &Forward, vec3 &Right, vec3 & Up)
{
	mat4 rotMat = MakeRotate(Rotation);

	Forward = vec3(rotMat[0][2], rotMat[1][2], rotMat[2][2]);
	Right = vec3(rotMat[0][0], rotMat[1][0], rotMat[2][0]);
	Up = vec3(rotMat[0][1], rotMat[1][1], rotMat[2][1]);
}

REGISTRATION
{
	RegisterNewType(vec2)
	->RegisterMember("x", &vec2::x)
	->RegisterMember("y", &vec2::y);

	RegisterNewType(vec3)
	->RegisterMember("x", &vec3::x)
	->RegisterMember("y", &vec3::y)
	->RegisterMember("z", &vec3::z);
}