#include <GameFramework/World.h>

class SMovement : public System
{
public:

	virtual void UpdateEntity(EntityID id, Space* space) override;

	virtual void SetSubscriptions() override;
};