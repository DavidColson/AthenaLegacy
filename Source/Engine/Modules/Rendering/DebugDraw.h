#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Scene.h"

namespace DebugDraw
{	
	struct DrawCall
	{
		int vertexCount{ 0 };
		int indexCount{ 0 };
	};
	
	struct DebugVertex
	{
		Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
		Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	};

	struct TransformData
	{
		Matrixf wvp;
	};

	void Draw2DCircle(Scene& scene, Vec2f pos, float radius, Vec3f color);
	void Draw2DLine(Scene& scene, Vec2f start, Vec2f end, Vec3f color);

	void OnDebugDrawStateAdded(Scene& scene, EntityID entity);
	void OnDebugDrawStateRemoved(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}

struct CDebugDrawingState
{
	std::vector<DebugDraw::DrawCall> drawQueue;
	std::vector<DebugDraw::DebugVertex> vertexList;
	std::vector<int> indexList;

	ProgramHandle debugShaderProgram;
	VertexBufferHandle vertexBuffer;
	int vertBufferSize = 0;
	IndexBufferHandle indexBuffer;
	int indexBufferSize = 0;

	ConstBufferHandle transformDataBuffer;
};