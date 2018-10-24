#include <GameFramework/World.h>

class SMovement : public System
{
public:

	virtual void UpdateEntity(EntityID id, Space* space, float deltaTime) override;

	virtual void SetSubscriptions() override;
};