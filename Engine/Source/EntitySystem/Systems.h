#pragma once

struct IComponent;
struct FrameContext;

class ISystem
{
public:
    virtual void Initialize() = 0;
	virtual void RegisterComponent(IComponent* pComponent) = 0;
	virtual void UnregisterComponent(IComponent* pComponent) = 0;
	virtual void Update(float deltaTime, FrameContext& ctx) = 0;
};