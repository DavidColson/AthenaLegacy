#include "SDL.h"
#include "SDL_syswm.h"

#include <comdef.h>
#include <vector>
#include <bitset>

#include "maths/maths.h"
#include "renderer.h"
#include "entity_system.h"

struct TransformComponent
{
	vec3 m_pos;
	float m_rot;
};

struct SimpleRotateComponent
{
	float m_rotSpeed{ 0.5f };
};

struct DrawableComponent
{
	RenderProxy* pRenderProxy;
};

class RotationSystem : public System
{
public:

	virtual void UpdateEntity(EntityID id) override
	{
		TransformComponent* pTransform = gGameWorld.GetComponent<TransformComponent>(id);
		SimpleRotateComponent* pRotate = gGameWorld.GetComponent<SimpleRotateComponent>(id);

		pTransform->m_rot += pRotate->m_rotSpeed;
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<TransformComponent>();
		Subscribe<SimpleRotateComponent>();
	}
};

class DrawPolygonSystem : public System 
{
public:
	virtual void StartEntity(EntityID id) override
	{
		TransformComponent* pTransform = gGameWorld.GetComponent<TransformComponent>(id);
		DrawableComponent* pDrawable = gGameWorld.GetComponent<DrawableComponent>(id);

		// Create a render proxy for this entity and submit it
		pDrawable->pRenderProxy = new RenderProxy(
			{
				Vertex(vec3(0.0f, 0.5f, 0.5f), color(1.0f, 0.0f, 0.0f)),
				Vertex(vec3(0.5f, -0.5f, 0.5f), color(0.0f, 1.0f, 0.0f)),
				Vertex(vec3(-0.5f, -0.5f, 0.5f), color(0.0f, 0.0f, 1.0f))
			}, {
				0, 1, 2, 0
			});
		gRenderer.SubmitProxy(pDrawable->pRenderProxy);

		pDrawable->pRenderProxy->SetTransform(pTransform->m_pos, pTransform->m_rot);
	}

	virtual void UpdateEntity(EntityID id) override
	{
		TransformComponent* pTransform = gGameWorld.GetComponent<TransformComponent>(id);
		DrawableComponent* pDrawable = gGameWorld.GetComponent<DrawableComponent>(id);
		
		pDrawable->pRenderProxy->SetTransform(pTransform->m_pos, pTransform->m_rot);
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<TransformComponent>();
		Subscribe<DrawableComponent>();
	}
};

int main(int argc, char *argv[])
{
	// Engine Init
	// ***********

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




	// Create our scene
	// ****************

	gGameWorld.RegisterSystem<RotationSystem>();
	gGameWorld.RegisterSystem<DrawPolygonSystem>();

	EntityID triangle = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<TransformComponent>(triangle);
	gGameWorld.AssignComponent<DrawableComponent>(triangle);
	gGameWorld.AssignComponent<SimpleRotateComponent>(triangle);

	EntityID triangle2 = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<TransformComponent>(triangle2)->m_pos = vec3(1.0f, 0.0f, 0.0f);
	gGameWorld.AssignComponent<DrawableComponent>(triangle2);

	EntityID triangle3 = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<TransformComponent>(triangle3)->m_pos = vec3(-1.0f, 0.0f, 0.0f);
	gGameWorld.AssignComponent<DrawableComponent>(triangle3);
	gGameWorld.AssignComponent<SimpleRotateComponent>(triangle3)->m_rotSpeed = 1.0f;




	// Main Loop
	// *********

	gGameWorld.StartSystems();

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