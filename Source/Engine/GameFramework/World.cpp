#include "GameFramework/World.h"

#include "Reflection.h"

int s_componentCounter = 0;
ComponentIdToTypeIdMap g_componentTypeMap;

REGISTRATION
{
	RegisterNewType(EntityID);
}