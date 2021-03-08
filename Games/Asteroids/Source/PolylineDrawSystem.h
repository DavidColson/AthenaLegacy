#pragma once

#include <Vec2.h>
#include <Vec4.h>
#include <Matrix.h>
#include <GraphicsDevice.h>
#include <Scene.h>
#include <Systems.h>
#include <Entity.h>
#include <SpatialComponent.h>

typedef eastl::fixed_vector<Vec2f, 50> VertsVector;
struct FrameContext;

struct IComponent;

struct Scene;
struct Mesh;
struct FrameContext;

struct Polyline : public SpatialComponent
{
	eastl::fixed_vector<Vec2f, 15> points;
	float thickness = 5.0f;
	bool connected = true;
	
	REFLECT_DERIVED()
};

struct PolylineDrawSystem : public ISystem
{
	~PolylineDrawSystem();

    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Draw(float deltaTime, FrameContext& ctx) override;

private:
	void AddPolyLine(const VertsVector& verts, float thickness, Vec4f color, bool connected);

	eastl::vector<Polyline*> polylineComponents;

	struct DrawCall
	{
		int vertexCount{ 0 };
		int indexCount{ 0 };
	};
	eastl::vector<DrawCall> drawQueue;
	eastl::vector<Vec3f> vertexList;
	eastl::vector<Vec4f> colorsList;
	eastl::vector<uint32_t> indexList;

	ProgramHandle shaderProgram;
	VertexBufferHandle vertexBuffer;
	int vertBufferSize = 0;
	VertexBufferHandle colorsBuffer;
	int colorsBufferSize = 0;
	IndexBufferHandle indexBuffer;
	int indexBufferSize = 0;

	ConstBufferHandle transformDataBuffer;
};
