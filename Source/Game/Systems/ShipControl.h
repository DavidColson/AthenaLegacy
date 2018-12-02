#pragma once

#include <GameFramework/World.h>

class SShipControl : public System
{
public:

	virtual void UpdateEntity(EntityID id, Space* space, float deltaTime) override;

	virtual void SetSubscriptions() override;
};
