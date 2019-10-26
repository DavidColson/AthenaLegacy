#include "Scene.h"
int s_componentCounter = 0;

REFLECT_BEGIN(CName)
REFLECT_MEMBER(name)
REFLECT_END()

REFLECT_BEGIN(CTransform)
REFLECT_MEMBER(pos)
REFLECT_MEMBER(rot)
REFLECT_MEMBER(sca)
REFLECT_MEMBER(vel)
REFLECT_END()
