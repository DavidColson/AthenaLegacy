#pragma once

#include <Entity.h>
#include <Systems.h>

class World;
struct AsteroidSpawnData;
struct PlayerComponent;

struct AsteroidSpawner : public IWorldSystem
{
    virtual void Activate() override {}

    virtual void Deactivate() override {}

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Update(UpdateContext& ctx) override;

private:
    AsteroidSpawnData* pSpawnData;
    PlayerComponent* pPlayer;
};