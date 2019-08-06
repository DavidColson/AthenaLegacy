#pragma once

#include "Maths/Vec3.h"
#include "Maths/Vec2.h"

#include <vector>

struct ID3D11Buffer;

struct Vertex
{
	Vertex(Vec3f pos) : m_pos(pos) {}

	Vec3f m_pos{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f m_col{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec2f m_texCoords{ Vec2f(0.0f, 0.0f) };
};

struct RenderProxy
{
	RenderProxy() {};
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices);

	void Draw();

	void SetTransform(Vec3f pos, float rot, Vec3f sca) { m_pos = pos; m_rot = rot; m_sca = sca; }

	ID3D11Buffer* m_pWVPBuffer;

	Vec3f m_pos{ Vec3f(0,0,0) };
	Vec3f m_sca{ Vec3f(1.f, 1.f, 1.f) };
	float m_rot{ 0.0f };
	float m_lineThickness{ 1.0f };

	ID3D11Buffer* m_pVertBuffer{ nullptr };
	std::vector<Vertex> m_vertices;

	ID3D11Buffer* m_pIndexBuffer{ nullptr };
	std::vector<int> m_indices;

	// draw states
};