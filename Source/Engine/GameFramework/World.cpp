#include "GameFramework/World.h"

#include "Reflection.h"
#include "TypeData.h"

int s_componentCounter = 0;
ComponentIdToTypeIdMap* ComponentIdToTypeIdMap::pInstance{ nullptr };

REGISTER(EntityID)
{
	NewType(EntityID);
}