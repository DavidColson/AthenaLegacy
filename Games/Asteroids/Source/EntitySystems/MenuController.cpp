#include "MenuController.h"

#include "Rendering/GameRenderer.h"
#include <Input/Input.h>
#include <SDL_scancode.h>

#include "../Components.h"
#include "../Asteroids.h"

void MenuController::Activate()
{
    
}

void MenuController::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<Polyline>())
	{
		pGraphics = static_cast<Polyline*>(pComponent);
	}
    if (pComponent->GetTypeData() == TypeDatabase::Get<MenuCursorComponent>())
	{
		pCursor = static_cast<MenuCursorComponent*>(pComponent);
	}
}

void MenuController::UnregisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<Polyline>())
	{
		pGraphics = nullptr;
	}
    if (pComponent->GetTypeData() == TypeDatabase::Get<MenuCursorComponent>())
	{
		pCursor = nullptr;
	}
}

void MenuController::Update(UpdateContext& ctx)
{
    const float w = GameRenderer::GetWidth();
    const float h = GameRenderer::GetHeight();
    float validPositions[] = { h / 2.0f + 18.0f, h / 2.0f - 62.0f};

    if (Input::GetKeyDown(SDL_SCANCODE_UP) && pCursor->currentState == MenuCursorComponent::Quit)
    {
        pCursor->currentState = MenuCursorComponent::Start;
    }
    if (Input::GetKeyDown(SDL_SCANCODE_DOWN) && pCursor->currentState == MenuCursorComponent::Start)
    {
        pCursor->currentState = MenuCursorComponent::Quit;
    }
    Vec3f pos = pGraphics->GetLocalPosition();
    pos.y = validPositions[pCursor->currentState];
    pGraphics->SetLocalPosition(pos); 

    if (Input::GetKeyDown(SDL_SCANCODE_RETURN))
    {
        switch (pCursor->currentState)
        {
        case MenuCursorComponent::Start:
            LoadMainScene();
            break;
        case MenuCursorComponent::Quit:
            Engine::StartShutdown();
            break;
        default:
            break;
        }
    }
}