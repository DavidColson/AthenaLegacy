#include "SDL.h"
#include "SDL_syswm.h"

#include <comdef.h>
#include <vector>
#include <bitset>

#include "maths/maths.h"
#include "renderer.h"
#include "entity_system.h"

struct Transform
{
	vec3 pos;
};

struct Shape
{
	vec3 color;
};

class MovementSystem : public System
{
public:

	virtual void UpdateEntity(EntityID id) override
	{
		Transform* pTransform = gGameWorld.GetComponent<Transform>(id);

		pTransform->pos.x += 0.01f;
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<Transform>();
	}
};


int main(int argc, char *argv[])
{
	gGameWorld.RegisterSystem<MovementSystem>();

	EntityID triangle = gGameWorld.NewEntity();
	Transform* pTransform = gGameWorld.AssignComponent<Transform>(triangle);
	Shape* pShape = gGameWorld.AssignComponent<Shape>(triangle);

	EntityID circle = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<Shape>(circle);

	pTransform->pos.x = 7.0f;
	pShape->color = vec3(0.5f, 1.0f, 0.0f);


	SDL_Init(SDL_INIT_VIDEO);


	float width = 640.0f;
	float height = 480.0f;

	SDL_Window *window = SDL_CreateWindow(
		"DirectX",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(width),
		int(height),
		0
	);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	gRenderer.Initialize(hwnd, width, height);

	// submit an object for rendering
	RenderProxy triangleProxy(
	{
		Vertex(vec3(0.0f, 0.5f, 0.5f), color(1.0f, 0.0f, 0.0f)),
		Vertex(vec3(0.5f, -0.5f, 0.5f), color(0.0f, 1.0f, 0.0f)),
		Vertex(vec3(-0.5f, -0.5f, 0.5f), color(0.0f, 0.0f, 1.0f))
	}, {
		0, 1, 2, 0
	});
	gRenderer.m_renderProxies.push_back(triangleProxy);

	SDL_Event event;
	bool bContinue = true;
	while (bContinue)
	{
		gGameWorld.UpdateSystems();

		gRenderer.RenderFrame();

		if (SDL_PollEvent(&event)) {
			/* an event was found */
			switch (event.type) {
			case SDL_QUIT:
				bContinue = false;
				break;
			}
		}
	}

	gRenderer.Shutdown();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}