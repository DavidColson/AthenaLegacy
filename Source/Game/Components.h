#pragma once

#include <TypeSystem.h>
#include <Maths/Vec2.h>
#include <Maths/Vec3.h>
#include <Renderer/RenderProxy.h>
#include <Log.h>
#include <Scene.h>


struct CTransform
{
	Vec3f pos;
	float rot;
	Vec3f sca{ Vec3f(1.f, 1.f, 1.f) };
	Vec3f vel{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f accel{ Vec3f(0.0f, 0.0f, 0.0f) };

	REFLECT()
};

struct CDrawable
{
	RenderProxy renderProxy;
	float lineThickness{ 1.5f };

	REFLECT()
};

struct CText
{
	std::string text;
	bool visible = true;

	REFLECT()
};

struct CPlayerControl
{
	float thrust{ 160.f };
	float rotateSpeed{ 0.1f };
	float dampening{ 0.f };
	float respawnTimer{ 0.0f };
	int 	lives = 3;
	// #RefactorNote: move this into the singleton HUD component
	EntityID lifeEntities[3]; // Stores the entityID of the three lives living in the corner of the screen
	// #RefactorNote Store an age timer, and give the player some invincibility when they spawn

	REFLECT()
};

struct CGameOver
{
};

struct CPlayerScore
{
	int score;
};

struct CBullet
{
	float speed = 300.0f;

	REFLECT()
};

struct CCollidable
{
	float radius{ 40.f };

	REFLECT()
};

struct CAsteroid
{
	int hitCount{ 0 };

	REFLECT()
};
