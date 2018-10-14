#ifndef RENDER_PROXY_
#define RENDER_PROXY_

#include <vector>

#include "maths/maths.h"

struct ID3D11Buffer;

struct Vertex
{
	Vertex(vec3 pos, vec3 col) : pos(pos), col(col) {}

	vec3 pos{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 col{ vec3(0.0f, 0.0f, 0.0f) };
};

class RenderProxy
{
public:
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices);

	void Draw();

	void SetTransform(vec3 pos, float rot) { m_pos = pos; m_rot = rot; }

private:
	ID3D11Buffer* m_wvpBuffer;

	vec3 m_pos;
	float m_rot;

	ID3D11Buffer* m_vertBuffer;
	std::vector<Vertex> m_vertices;

	ID3D11Buffer* m_indexBuffer;
	std::vector<int> m_indices;

	// draw states
};
#endif