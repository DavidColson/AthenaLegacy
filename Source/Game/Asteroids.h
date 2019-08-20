#include <vector>

#include <Renderer/RenderProxy.h>

namespace Game
{
  extern std::vector<RenderProxy> g_asteroidMeshes;
	extern RenderProxy g_shipMesh;

	void Startup();

	void Update(float deltaTime);
}