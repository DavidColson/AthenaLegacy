#pragma once

#include "Maths/Vec3.h"
#include "Maths/Vec2.h"
#include "GraphicsDevice/GraphicsDevice.h"

#include <vector>

struct RenderProxy
{
	RenderProxy() {};
	RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices, const std::string& name);

	void Draw();

	void SetTransform(Vec3f _pos, float _rot, Vec3f _sca) { pos = _pos; rot = _rot; sca = _sca; }

	ConstBufferHandle transformBuffer;

	Vec3f pos{ Vec3f(0,0,0) };
	Vec3f sca{ Vec3f(1.f, 1.f, 1.f) };
	float rot{ 0.0f };
	float lineThickness{ 1.0f };

	std::string debugName;

	VertexBufferHandle vertBuffer;
	IndexBufferHandle indexBuffer;
};