#include <vector>

#include <Renderer/RenderProxy.h>

namespace Game
{
	extern std::vector<RenderProxy> g_asteroidMeshes;

	void Startup();

	void Update(float deltaTime);
}