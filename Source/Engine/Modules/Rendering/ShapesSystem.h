#pragma once

#include "Vec2.h"
#include "Matrix.h"
#include "GraphicsDevice.h"
#include "Scene.h"

namespace Shapes
{	
	struct DrawCall
	{
		int vertexCount{ 0 };
		int indexCount{ 0 };
	};
	
	struct ShapeVertex
	{
		Vec2f pos{ Vec2f(0.0f, 0.0f) };
		Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	};

	struct TransformData
	{
		Matrixf wvp;
		float lineThickness{ 5.0f };
		float pad1{ 0.0f };
		float pad2{ 0.0f };
		float pad3{ 0.0f };
	};

	void DrawPolyLine(Scene& scene, const std::vector<Vec2f>& verts, float thickness, Vec3f color, bool connected = true);

	void OnShapesSystemStateAdded(Scene& scene, EntityID entity);
	void OnShapesSystemStateRemoved(Scene& scene, EntityID entity);
	void OnFrame(Scene& scene, float deltaTime);
}

struct CShapesSystemState
{
	std::vector<Shapes::DrawCall> drawQueue;
	std::vector<Shapes::ShapeVertex> vertexList;
	std::vector<int> indexList;

	ProgramHandle shaderProgram;
	VertexBufferHandle vertexBuffer;
	int vertBufferSize = 0;
	IndexBufferHandle indexBuffer;
	int indexBufferSize = 0;

	ConstBufferHandle transformDataBuffer;
};