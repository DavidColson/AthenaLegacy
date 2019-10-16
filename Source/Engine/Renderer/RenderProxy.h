#pragma once

#include "Maths/Vec3.h"
#include "Maths/Vec2.h"

#include <vector>

struct ID3D11Buffer;

struct Vertex
{
	Vertex(Vec3f pos) : pos(pos) {}

	Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec2f texCoords{ Vec2f(0.0f, 0.0f) };
};

struct RenderProxy
{
	RenderProxy() {};
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices);

	void Draw();

	void SetTransform(Vec3f pos, float rot, Vec3f sca) { pos = pos; rot = rot; sca = sca; }

	ID3D11Buffer* pWVPBuffer;

	Vec3f pos{ Vec3f(0,0,0) };
	Vec3f sca{ Vec3f(1.f, 1.f, 1.f) };
	float rot{ 0.0f };
	float lineThickness{ 1.0f };

	ID3D11Buffer* pVertBuffer{ nullptr };
	std::vector<Vertex> vertices;

	ID3D11Buffer* pIndexBuffer{ nullptr };
	std::vector<int> indices;

	// draw states
};