#include "SDL.h"
#include "SDL_syswm.h"

#include <comdef.h>
#include <vector>
#include <bitset>

#include "GameFramework/World.h"
#include "Input/Input.h"
#include "Maths/Maths.h"
#include "Renderer/Renderer.h"

struct CTransform
{
	vec3 m_pos;
	float m_rot;
};

struct CSimpleRotate
{
	float m_rotSpeed{ 0.5f };
};

struct CDrawable
{
	RenderProxy m_renderProxy;
};

struct CPlayerControl
{
	vec3 m_moveSpeed{ vec3(0.02f, 0.02f, 0.02f) };
};

class SRotation : public System
{
public:

	virtual void UpdateEntity(EntityID id) override
	{
		CTransform* pTransform = gGameWorld.GetComponent<CTransform>(id);
		CSimpleRotate* pRotate = gGameWorld.GetComponent<CSimpleRotate>(id);

		pTransform->m_rot += pRotate->m_rotSpeed;
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<CTransform>();
		Subscribe<CSimpleRotate>();
	}
};

class SMovement : public System
{
public:

	virtual void UpdateEntity(EntityID id) override
	{
		CTransform* pTransform = gGameWorld.GetComponent<CTransform>(id);
		CPlayerControl* pControl = gGameWorld.GetComponent<CPlayerControl>(id);
		if (g_Input.GetKeyHeld(SDL_SCANCODE_D))
			pTransform->m_pos.x += pControl->m_moveSpeed.x;
		if (g_Input.GetKeyHeld(SDL_SCANCODE_A))
			pTransform->m_pos.x -= pControl->m_moveSpeed.x;
		if (g_Input.GetKeyHeld(SDL_SCANCODE_W))
			pTransform->m_pos.y += pControl->m_moveSpeed.y;
		if (g_Input.GetKeyHeld(SDL_SCANCODE_S))
			pTransform->m_pos.y -= pControl->m_moveSpeed.y;
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<CTransform>();
		Subscribe<CPlayerControl>();
	}
};

class SDrawPolygon : public System 
{
public:
	virtual void StartEntity(EntityID id) override
	{
		CTransform* pTransform = gGameWorld.GetComponent<CTransform>(id);
		CDrawable* pDrawable = gGameWorld.GetComponent<CDrawable>(id);

		// Create a render proxy for this entity and submit it
		pDrawable->m_renderProxy = RenderProxy(
			{
				Vertex(vec3(0.0f, 0.5f, 0.5f), color(1.0f, 0.0f, 0.0f)),
				Vertex(vec3(0.5f, -0.5f, 0.5f), color(0.0f, 1.0f, 0.0f)),
				Vertex(vec3(-0.5f, -0.5f, 0.5f), color(0.0f, 0.0f, 1.0f))
			}, {
				0, 1, 2, 0
			});
		g_Renderer.SubmitProxy(&pDrawable->m_renderProxy);

		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot);
	}

	virtual void UpdateEntity(EntityID id) override
	{
		CTransform* pTransform = gGameWorld.GetComponent<CTransform>(id);
		CDrawable* pDrawable = gGameWorld.GetComponent<CDrawable>(id);
		
		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot);
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<CTransform>();
		Subscribe<CDrawable>();
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

	g_Renderer.Initialize(hwnd, width, height);




	// Create our scene
	// ****************

	gGameWorld.RegisterSystem<SRotation>();
	gGameWorld.RegisterSystem<SDrawPolygon>();
	gGameWorld.RegisterSystem<SMovement>();

	EntityID triangle = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<CTransform>(triangle);
	gGameWorld.AssignComponent<CDrawable>(triangle);
	gGameWorld.AssignComponent<CSimpleRotate>(triangle);

	EntityID triangle2 = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<CTransform>(triangle2)->m_pos = vec3(1.0f, 0.0f, 0.0f);
	gGameWorld.AssignComponent<CDrawable>(triangle2);

	EntityID triangle3 = gGameWorld.NewEntity();
	gGameWorld.AssignComponent<CTransform>(triangle3)->m_pos = vec3(-1.0f, 0.0f, 0.0f);
	gGameWorld.AssignComponent<CDrawable>(triangle3);
	gGameWorld.AssignComponent<CPlayerControl>(triangle3);






	// Main Loop
	// *********

	gGameWorld.StartSystems();

	bool shutdown = false;
	while (!shutdown)
	{
		g_Input.Update(shutdown);

		gGameWorld.UpdateSystems();

		g_Renderer.RenderFrame();
	}

	g_Renderer.Shutdown();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}