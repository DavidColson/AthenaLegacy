#pragma once

#include "Maths/Maths.h"

#include <vector>

struct ID3D11Buffer;

struct Vertex
{
	Vertex(vec3 pos, vec3 col) : m_pos(pos), m_col(col) {}

	vec3 m_pos{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 m_col{ vec3(0.0f, 0.0f, 0.0f) };
};

class RenderProxy
{
public:
	RenderProxy() {};
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices);

	void Draw();

	void SetTransform(vec3 pos, float rot) { m_pos = pos; m_rot = rot; }

private:
	ID3D11Buffer* m_pWVPBuffer;

	vec3 m_pos{ vec3(0,0,0) };
	float m_rot{ 0.0f };

	ID3D11Buffer* m_pVertBuffer{ nullptr };
	std::vector<Vertex> m_vertices;

	ID3D11Buffer* m_pIndexBuffer{ nullptr };
	std::vector<int> m_indices;

	// draw states
};