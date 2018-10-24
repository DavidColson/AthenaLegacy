#include <GameFramework/World.h>

class SDrawPolygon : public System
{
public:
	virtual void StartEntity(EntityID id, Space* space) override;

	virtual void UpdateEntity(EntityID id, Space* space) override;

	virtual void SetSubscriptions() override;
};
