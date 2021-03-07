#pragma once

struct IComponent;
struct FrameContext;

class ISystem
{
public:
    virtual void Activate() = 0;
	virtual void RegisterComponent(IComponent* pComponent) = 0;
	virtual void UnregisterComponent(IComponent* pComponent) = 0;

	virtual void Update(float deltaTime) {};
	virtual void Draw(float deltaTime, FrameContext& ctx) {};
};