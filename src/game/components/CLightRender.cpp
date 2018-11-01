/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/client/CClient.hpp>
#include "CLightRender.hpp"
#include <cstring>

CLightRender::CLightRender(CGameClient *pGameClient, int render) noexcept
: CComponent(pGameClient)
{
	m_Render = render;
}
CLightRender::~CLightRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CLightRender", "Deleted");
	#endif
}

void CLightRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->getRenderMode() != RENDER_MODE_LIGHTING && Client()->getRenderMode() != RENDER_MODE_NORMAL)
		return;

	target.setView(Client()->Camera());

	std::vector<CEntity*> &vEntities = Client()->Controller()->Context()->getEntities();
	std::vector<CEntity*>::const_iterator cit = vEntities.cbegin();
	while (cit != vEntities.cend())
	{
		if ((*cit)->getType() == CEntity::LIGHT)
		{
			CLight *pLight = static_cast<CLight*>(*cit);
			renderLight(target, states, pLight);
		}
		++cit;
	}
}

void CLightRender::renderLight(sf::RenderTarget& target, sf::RenderStates states, CLight *pLight) const noexcept
{
	if (pLight->m_Active && !pLight->m_Blinked)
	{
		if (Client()->isClipped(pLight->m_Position, SCREEN_MARGIN_DRAW))
			return;

		pLight->m_Sprite.setRotation(pLight->m_Degrees);
		const sf::Texture *pText = pLight->m_Sprite.getTexture();
		pLight->m_Sprite.setOrigin(pText->getSize().x*pLight->m_Origin.x, pText->getSize().y*pLight->m_Origin.y);
		float factorSize = 1.0f;
		if (pLight->m_VariationSize != 0.0f)
			factorSize = upm::floatRand(1.0f, pLight->m_VariationSize);
		pLight->m_Sprite.setScale(pLight->m_Scale*factorSize);
		pLight->m_Sprite.setColor(pLight->m_Color);
		pLight->m_Sprite.setPosition(pLight->m_Position - pLight->m_Scale/2.0f);
		//pLight->m_Sprite.setRotation(pLight->m_Degrees);
		states.blendMode = sf::BlendAdd;
		target.draw(pLight->m_Sprite, states);
	}
}
