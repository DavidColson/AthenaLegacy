#include "DrawPolygon.h"

#include "Components/Components.h"

#include <Renderer/Renderer.h>

void SDrawPolygon::StartEntity(EntityID id, Space * space)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);
	CDrawable* pDrawable = space->GetComponent<CDrawable>(id);

	Graphics::SubmitProxy(&pDrawable->m_renderProxy);

	pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
}

void SDrawPolygon::UpdateEntity(EntityID id, Space * space, float deltaTime)
{
	CTransform* pTransform = space->GetComponent<CTransform>(id);
	CDrawable* pDrawable = space->GetComponent<CDrawable>(id);

	pDrawable->m_renderProxy.SetTransform(pTransform->m_pos, pTransform->m_rot, pTransform->m_sca);
	pDrawable->m_renderProxy.m_lineThickness = pDrawable->m_lineThickness;
}

void SDrawPolygon::SetSubscriptions()
{
	Subscribe<CTransform>();
	Subscribe<CDrawable>();
}
