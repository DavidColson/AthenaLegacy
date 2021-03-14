#include <Scene.h>

#include <EASTL/fixed_vector.h>

#define PLAYER_ID EntityID{ 0 }

void LoadMainScene();
void LoadMenu();

eastl::fixed_vector<Vec2f, 15> GetRandomAsteroidMesh();