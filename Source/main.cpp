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
	vec3 m_sca{ vec3(1.f, 1.f, 1.f) };
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
	vec3 m_moveSpeed{ vec3(5.f, 5.f, 5.f) };
};

class SRotation : public System
{
public:

	virtual void UpdateEntity(EntityID id, Space* space) override
	{
		CTransform* pTransform = space->GetComponent<CTransform>(id);
		CSimpleRotate* pRotate = space->GetComponent<CSimpleRotate>(id);

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

	virtual void UpdateEntity(EntityID id, Space* space) override
	{
		CTransform* pTransform = space->GetComponent<CTransform>(id);
		CPlayerControl* pControl = space->GetComponent<CPlayerControl>(id);
		if (Input::GetKeyHeld(SDL_SCANCODE_D))
			pTransform->m_pos.x += pControl->m_moveSpeed.x;
		if (Input::GetKeyHeld(SDL_SCANCODE_A))
			pTransform->m_pos.x -= pControl->m_moveSpeed.x;
		if (Input::GetKeyHeld(SDL_SCANCODE_W))
			pTransform->m_pos.y += pControl->m_moveSpeed.y;
		if (Input::GetKeyHeld(SDL_SCANCODE_S))
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
	virtual void StartEntity(EntityID id, Space* space) override
	{
		CTransform* pTransform = space->GetComponent<CTransform>(id);
		CDrawable* pDrawable = space->GetComponent<CDrawable>(id);

		assert(pDrawable->m_renderProxy.m_pWVPBuffer != nullptr);
		Graphics::SubmitProxy(&pDrawable->m_renderProxy);

		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
	}

	virtual void UpdateEntity(EntityID id, Space* space) override
	{
		CTransform* pTransform = space->GetComponent<CTransform>(id);
		CDrawable* pDrawable = space->GetComponent<CDrawable>(id);
		
		pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
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

	float width = 1000.0f;
	float height = 600.0f;

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


	Graphics::CreateContext(hwnd, width, height);
	Input::CreateInputState();



	// Create our scene
	// ****************
	Space* currentSpace = new Space();

	currentSpace->RegisterSystem<SRotation>();
	currentSpace->RegisterSystem<SDrawPolygon>();
	currentSpace->RegisterSystem<SMovement>();

	EntityID asteroid = currentSpace->NewEntity();
	currentSpace->AssignComponent<CTransform>(asteroid)->m_pos = vec3(100.0f, 100.0f, 0.0f);
	currentSpace->AssignComponent<CDrawable>(asteroid)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(49.6f, 90.6f, 0.5f), color(1.0f, 0.0f, 0.0f)),
			Vertex(vec3(39.f, 51.6f, 0.5f), color(0.0f, 1.0f, 0.0f)),
			Vertex(vec3(19.6f, 54.2f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(5.2f, 31.7f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(24.2f, 8.3f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(51.f, 20.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(97.f, 18.7f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(75.3f, 45.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(95.f, 76.4f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		}, {
			0, 1, 2, 3, 4, 5, 6, 7, 8, 0
		});


	EntityID ship = currentSpace->NewEntity();
	CTransform* pTransform = currentSpace->AssignComponent<CTransform>(ship);
	
	pTransform->m_pos = vec3(500.0f, 300.0f, 0.0f);
	pTransform->m_sca = vec3(0.3f, 0.3f, 1.0f);
	
	currentSpace->AssignComponent<CPlayerControl>(ship);
	currentSpace->AssignComponent<CDrawable>(ship)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(0.f, 50.f, 0.5f), color(1.0f, 0.0f, 0.0f)),
			Vertex(vec3(100.f, 80.f, 0.5f), color(0.0f, 1.0f, 0.0f)),
			Vertex(vec3(85.f, 70.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(85.f, 30.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(100.f, 20.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		}, {
			0, 1, 2, 3, 4, 0
		});





	// Main Loop
	// *********

	currentSpace->StartSystems();

	bool shutdown = false;
	while (!shutdown)
	{
		Input::Update(shutdown);

		currentSpace->UpdateSystems();

		Graphics::RenderFrame();

	}

	Graphics::Shutdown();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}