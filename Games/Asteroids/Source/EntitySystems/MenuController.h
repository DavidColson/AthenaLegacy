#pragma once

#include <Entity.h>
#include <Systems.h>

class World;
struct UpdateContext;
struct MenuCursorComponent;
struct Polyline;

struct MenuController : public IEntitySystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:
    MenuCursorComponent* pCursor;
    Polyline* pGraphics;
};