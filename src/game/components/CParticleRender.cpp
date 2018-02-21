/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CAssetManager.hpp>
#include <engine/CGame.hpp>
#include <game/CGameClient.hpp>
#include "CParticleRender.hpp"


CParticleRender::CParticleRender(CGameClient *pGameClient, int render) noexcept
: CComponent(pGameClient)
{
	m_Render = render;
}
CParticleRender::~CParticleRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CParticleRender", "Deleted");
	#endif
}

void CParticleRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	target.setView(Client()->Camera());

	std::vector<CEntity*> &vEntities = Client()->Controller()->Context()->getEntities();
	std::vector<CEntity*>::const_iterator cit = vEntities.cbegin();
	while (cit != vEntities.cend())
	{
		if ((*cit)->getType() == CEntity::PARTICLE)
		{
			CParticle *pParticle = static_cast<CParticle*>(*cit);
			if (pParticle->m_Render == m_Render &&
					(Client()->getRenderMode() == CGameClient::RENDER_NORMAL ||
							(Client()->getRenderMode() == CGameClient::RENDER_LIGHTING && pParticle->m_Luminance)))
			{
				renderParticle(target, states, pParticle);
			}
		}

		++cit;
	}
}

void CParticleRender::renderParticle(sf::RenderTarget& target, sf::RenderStates states, CParticle *pParticle) const noexcept
{
	const float elapsedSeconds = (float)(ups::timeGet()-pParticle->m_Timer)/ups::timeFreq();
    const sf::Vector2f sizeStep(((pParticle->m_SizeEnd.x-pParticle->m_SizeInit.x)/pParticle->m_Duration)*elapsedSeconds, ((pParticle->m_SizeEnd.y-pParticle->m_SizeInit.y)/pParticle->m_Duration)*elapsedSeconds);
    const sf::Vector3f colorStep(((pParticle->m_ColorEnd.r-pParticle->m_ColorInit.r)/pParticle->m_Duration)*elapsedSeconds, ((pParticle->m_ColorEnd.g-pParticle->m_ColorInit.g)/pParticle->m_Duration)*elapsedSeconds, ((pParticle->m_ColorEnd.b-pParticle->m_ColorInit.b)/pParticle->m_Duration)*elapsedSeconds);
    const float colorAlphaStep = ((pParticle->m_ColorEnd.a-pParticle->m_ColorInit.a)/pParticle->m_Duration)*elapsedSeconds;

	if (!pParticle->m_ModeText)
	{
		sf::RectangleShape Shape;
		Shape.setTexture(Client()->Assets().getTexture(pParticle->m_TextId));

		if (!Client()->m_Paused)
			Shape.setRotation(pParticle->m_Rotation);
		if (pParticle->m_Animated)
		{
			const int animStep = ((pParticle->m_AnimSize.x*pParticle->m_AnimSize.y)/pParticle->m_Duration)*elapsedSeconds;
			const sf::Vector2i animStepPos = { animStep%pParticle->m_AnimSize.x, animStep/pParticle->m_AnimSize.x };
			const sf::Vector2i tileSize = { (int)(Shape.getTexture()->getSize().x/pParticle->m_AnimSize.x), (int)(Shape.getTexture()->getSize().y/pParticle->m_AnimSize.y) };
			Shape.setTextureRect(sf::IntRect(animStepPos.x*tileSize.x, animStepPos.y*tileSize.y, tileSize.x, tileSize.y));
		}
		Shape.setPosition(pParticle->m_Pos+(pParticle->m_FixedPos?Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f)));
		Shape.setSize(pParticle->m_SizeInit+sizeStep);
		Shape.setOrigin((pParticle->m_SizeInit.x+sizeStep.x)/2.0f, (pParticle->m_SizeInit.y+sizeStep.y)/2.0f);
		Shape.setFillColor(sf::Color(pParticle->m_ColorInit.r+colorStep.x, pParticle->m_ColorInit.g+colorStep.y, pParticle->m_ColorInit.b+colorStep.z, pParticle->m_ColorInit.a+colorAlphaStep));
		target.draw(Shape, states);
	}
	else
	{
		sf::Text Text;
		Text.setFont(Client()->Assets().getDefaultFont());
		if (!Client()->m_Paused)
			Text.setRotation(pParticle->m_Rotation);
		Text.setPosition(pParticle->m_Pos+(pParticle->m_FixedPos?Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f)));
		Text.setScale((pParticle->m_SizeInit.x+sizeStep.x)/pParticle->m_SizeInit.x, (pParticle->m_SizeInit.y+sizeStep.y)/pParticle->m_SizeInit.y);
		Text.setOrigin(Text.getLocalBounds().width/2.0f, Text.getLocalBounds().height/2.0f);
		Text.setFillColor(sf::Color(pParticle->m_ColorInit.r+colorStep.x, pParticle->m_ColorInit.g+colorStep.y, pParticle->m_ColorInit.b+colorStep.z, pParticle->m_ColorInit.a+colorAlphaStep));
		target.draw(Text, states);
	}
}
