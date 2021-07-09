#include "Components.h"

REFLECT_BEGIN_DERIVED(AsteroidComponent, IComponent)
REFLECT_MEMBER(hitCount)
REFLECT_END()

REFLECT_BEGIN_DERIVED(AsteroidPhysics, SpatialComponent)
REFLECT_MEMBER(velocity)
REFLECT_MEMBER(acceleration)
REFLECT_MEMBER(collisionRadius)
REFLECT_END()

REFLECT_BEGIN_DERIVED(Polyline, SpatialComponent)
REFLECT_MEMBER(thickness)
REFLECT_MEMBER(connected)
REFLECT_MEMBER(visible)
REFLECT_END()

REFLECT_BEGIN_DERIVED(PlayerComponent, IComponent)
REFLECT_MEMBER(thrust)
REFLECT_MEMBER(rotateSpeed)
REFLECT_MEMBER(dampening)
REFLECT_MEMBER(respawnTimer)
REFLECT_MEMBER(lives)
REFLECT_END()

REFLECT_BEGIN_DERIVED(Score, IComponent)
REFLECT_MEMBER(currentScore)
REFLECT_MEMBER(highScore)
REFLECT_END()

REFLECT_BEGIN_DERIVED(AsteroidSpawnData, IComponent)
REFLECT_MEMBER(timer)
REFLECT_MEMBER(timeBetweenSpawns)
REFLECT_MEMBER(decay)
REFLECT_END()

REFLECT_BEGIN_DERIVED(MenuCursorComponent, IComponent)
REFLECT_END()
