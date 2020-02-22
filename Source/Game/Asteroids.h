#include <vector>

#include <Renderer/Renderer.h>
#include <Scene.h>

#define PLAYER_ID CreateEntityId(1, 0)

struct Shape
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;
};

namespace Game
{
  	extern std::vector<Shape> g_asteroidMeshes;
 	extern Shape g_shipMesh;
}