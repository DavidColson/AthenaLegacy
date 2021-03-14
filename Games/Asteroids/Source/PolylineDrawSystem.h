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
struct Polyline;

struct PolylineDrawSystem : public IWorldSystem
{
	~PolylineDrawSystem();

    virtual void Activate() override;

    virtual void Deactivate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) override;

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
