#pragma once

struct IComponent;
struct FrameContext;
struct UpdateContext;

class ISystem
{
public:
    virtual void Activate() = 0;
	virtual void RegisterComponent(IComponent* pComponent) = 0;
	virtual void UnregisterComponent(IComponent* pComponent) = 0;

	virtual void Update(UpdateContext& ctx) {};
	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) {};
};