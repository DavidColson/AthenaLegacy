#pragma once

#include "Maths/Maths.h"

#include <vector>

struct ID3D11Buffer;

struct Vertex
{
	Vertex(vec3 pos) : m_pos(pos) {}

	vec3 m_pos{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 m_col{ vec3(0.0f, 0.0f, 0.0f) };
	vec2 m_texCoords{ vec2(0.0f, 0.0f) };
};

struct RenderProxy
{
	RenderProxy() {};
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices);

	void Draw();

	void SetTransform(vec3 pos, float rot, vec3 sca) { m_pos = pos; m_rot = rot; m_sca = sca; }

	ID3D11Buffer* m_pWVPBuffer;

	vec3 m_pos{ vec3(0,0,0) };
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };
	float m_rot{ 0.0f };
	float m_lineThickness{ 1.0f };

	ID3D11Buffer* m_pVertBuffer{ nullptr };
	std::vector<Vertex> m_vertices;

	ID3D11Buffer* m_pIndexBuffer{ nullptr };
	std::vector<int> m_indices;

	// draw states
};