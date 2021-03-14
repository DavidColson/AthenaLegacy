#include "UIUpdateSystem.h"

#include "../Components.h"

void UIUpdateSystem::Activate()
{

}

void UIUpdateSystem::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<TextComponent>())
    {
        textElements[pComponent->GetId()] = static_cast<TextComponent*>(pComponent);
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<Score>())
    {
        pScoreComponent = static_cast<Score*>(pComponent);
    }
}   

void UIUpdateSystem::UnregisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<TextComponent>())
    {
        textElements.erase(pComponent->GetId());
    }

    if (pComponent->GetTypeData() == TypeDatabase::Get<Score>())
    {
        pScoreComponent = nullptr;
    }
}

void UIUpdateSystem::Update(UpdateContext& ctx)
{
    if (pScoreComponent)
    {

        if (pScoreComponent->update)
        {
            if (pScoreComponent->gameOver)
            {
                textElements[pScoreComponent->gameOverTextElement]->visible = true;
            }

            pScoreComponent->update = false;

            if (pScoreComponent->currentScore > pScoreComponent->highScore)
            {
                pScoreComponent->highScore = pScoreComponent->currentScore;
            }

            textElements[pScoreComponent->currentScoreTextElement]->text.sprintf("%i", pScoreComponent->currentScore);
            textElements[pScoreComponent->highScoreTextElement]->text.sprintf("%i", pScoreComponent->highScore);
        }
    }
}