#include "GameFramework/World.h"

#include "TypeDB.h"
#include "TypeData.h"

int s_componentCounter = 0;

REGISTER(EntityID)
{
	NewType(EntityID);
}