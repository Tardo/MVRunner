/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CAssetManager.hpp>
#include <engine/CGame.hpp>
#include <game/CGameClient.hpp>
#include "CSimpleParticleRender.hpp"


CSimpleParticleRender::CSimpleParticleRender(CGameClient *pGameClient, int render) noexcept
: CComponent(pGameClient)
{
	m_Render = render;
}
CSimpleParticleRender::~CSimpleParticleRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSimpleParticleRender", "Deleted");
	#endif
}

void CSimpleParticleRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->getRenderMode() != RENDER_MODE_LIGHTING && Client()->getRenderMode() != RENDER_MODE_NORMAL)
		return;

	target.setView(Client()->Camera());

	std::vector<CEntity*> &vEntities = Client()->Controller()->Context()->getEntities();
	std::vector<CEntity*>::const_iterator cit = vEntities.cbegin();
	while (cit != vEntities.cend())
	{
		if ((*cit)->getType() == CEntity::SIMPLE_PARTICLE)
		{
			CSimpleParticle *pSimpleParticle = static_cast<CSimpleParticle*>(*cit);
			if (pSimpleParticle->m_Render == m_Render &&
					((Client()->getRenderMode() == RENDER_MODE_NORMAL && !pSimpleParticle->m_Luminance) ||
							(Client()->getRenderMode() == RENDER_MODE_LIGHTING && pSimpleParticle->m_Luminance)))
			{
				renderSimpleParticle(target, states, pSimpleParticle);
			}
		}

		++cit;
	}
}

void CSimpleParticleRender::renderSimpleParticle(sf::RenderTarget& target, sf::RenderStates states, CSimpleParticle *pSimpleParticle) const noexcept
{
	const sf::Vector2f particlePos = pSimpleParticle->m_Pos+(pSimpleParticle->m_FixedPos?target.getView().getCenter():VECTOR_ZERO);
	if (Client()->isClipped(particlePos, SCREEN_MARGIN_DRAW))
		return;

	const float elapsedSeconds = upm::clamp((float)(ups::timeGet()-pSimpleParticle->m_Timer)/ups::timeFreq(), 0.0f, pSimpleParticle->m_Duration);
    const sf::Vector2f sizeStep(((pSimpleParticle->m_SizeEnd.x-pSimpleParticle->m_SizeInit.x)/pSimpleParticle->m_Duration)*elapsedSeconds, ((pSimpleParticle->m_SizeEnd.y-pSimpleParticle->m_SizeInit.y)/pSimpleParticle->m_Duration)*elapsedSeconds);
    const sf::Vector3f colorStep(((pSimpleParticle->m_ColorEnd.r-pSimpleParticle->m_ColorInit.r)/pSimpleParticle->m_Duration)*elapsedSeconds, ((pSimpleParticle->m_ColorEnd.g-pSimpleParticle->m_ColorInit.g)/pSimpleParticle->m_Duration)*elapsedSeconds, ((pSimpleParticle->m_ColorEnd.b-pSimpleParticle->m_ColorInit.b)/pSimpleParticle->m_Duration)*elapsedSeconds);
    const float colorAlphaStep = ((pSimpleParticle->m_ColorEnd.a-pSimpleParticle->m_ColorInit.a)/pSimpleParticle->m_Duration)*elapsedSeconds;

	if (!pSimpleParticle->m_ModeText)
	{
		sf::RectangleShape Shape;
		Shape.setTexture(Client()->Assets().getTexture(pSimpleParticle->m_TextId));

		if (!Client()->m_Paused)
			Shape.setRotation(pSimpleParticle->m_Rotation);
		if (pSimpleParticle->m_Animated)
		{
			const int animStep = ((pSimpleParticle->m_AnimSize.x*pSimpleParticle->m_AnimSize.y)/pSimpleParticle->m_Duration)*elapsedSeconds;
			const sf::Vector2i animStepPos = { animStep%pSimpleParticle->m_AnimSize.x, animStep/pSimpleParticle->m_AnimSize.x };
			const sf::Vector2i tileSize = { (int)(Shape.getTexture()->getSize().x/pSimpleParticle->m_AnimSize.x), (int)(Shape.getTexture()->getSize().y/pSimpleParticle->m_AnimSize.y) };
			Shape.setTextureRect(sf::IntRect(animStepPos.x*tileSize.x, animStepPos.y*tileSize.y, tileSize.x, tileSize.y));
		}
		Shape.setPosition(particlePos);
		Shape.setSize(pSimpleParticle->m_SizeInit+sizeStep);
		Shape.setOrigin((pSimpleParticle->m_SizeInit.x+sizeStep.x)/2.0f, (pSimpleParticle->m_SizeInit.y+sizeStep.y)/2.0f);
		Shape.setFillColor(sf::Color(pSimpleParticle->m_ColorInit.r+colorStep.x, pSimpleParticle->m_ColorInit.g+colorStep.y, pSimpleParticle->m_ColorInit.b+colorStep.z, pSimpleParticle->m_ColorInit.a+colorAlphaStep));
		target.draw(Shape, states);
	}
	else
	{
		sf::Text Text;
		Text.setFont(Client()->Assets().getDefaultFont());
		if (!Client()->m_Paused)
			Text.setRotation(pSimpleParticle->m_Rotation);
		Text.setPosition(pSimpleParticle->m_Pos+(pSimpleParticle->m_FixedPos?Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f)));
		Text.setScale((pSimpleParticle->m_SizeInit.x+sizeStep.x)/pSimpleParticle->m_SizeInit.x, (pSimpleParticle->m_SizeInit.y+sizeStep.y)/pSimpleParticle->m_SizeInit.y);
		Text.setOrigin(Text.getLocalBounds().width/2.0f, Text.getLocalBounds().height/2.0f);
		Text.setFillColor(sf::Color(pSimpleParticle->m_ColorInit.r+colorStep.x, pSimpleParticle->m_ColorInit.g+colorStep.y, pSimpleParticle->m_ColorInit.b+colorStep.z, pSimpleParticle->m_ColorInit.a+colorAlphaStep));
		target.draw(Text, states);
	}
}
