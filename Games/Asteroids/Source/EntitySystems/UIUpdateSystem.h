#pragma once

#include <Entity.h>
#include <Systems.h>
#include <Rendering/FontSystem.h>

struct UpdateContext;
struct Score;

class World;

struct UIUpdateSystem : public IEntitySystem
{
    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:
	eastl::map<Uuid, TextComponent*> textElements;

    Score* pScoreComponent;
};