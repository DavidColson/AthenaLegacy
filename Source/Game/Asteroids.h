#include <vector>

#include <Renderer/Renderer.h>

#define PLAYER_ID 0

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