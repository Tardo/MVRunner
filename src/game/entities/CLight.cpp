/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CLight.hpp"
#include <engine/client/CClient.hpp>

/** LIGHT **/
CLight::CLight(sf::Texture *pTexture, const sf::Vector2f &nposition, float angle, const sf::Vector2f &nscale, const sf::Color &ncolor, bool alwaysOn, float blink, float variationSize, int render) noexcept
: CEntity(CEntity::LIGHT)
{
	m_Position = nposition;
	m_Scale = nscale;
	m_Color = ncolor;
	m_Blink = blink;
	m_Degrees = angle;
	m_Render = render;
	m_AlwaysOn = alwaysOn;
	m_Origin = VECTOR_ZERO;
	m_Active = true;
	m_Blinked = false;
	m_TimerBlink = 0;
	m_VariationSize = variationSize;

	if (pTexture)
		m_Sprite.setTexture(*pTexture);
}
CLight::~CLight() noexcept
{ }

void CLight::tick() noexcept
{
	CClient *pGame = CClient::getInstance();
	if (pGame->Client()->isClipped(m_Position, SCREEN_MARGIN_DESTRUCTION))
		destroy();

	// Parpadeos
	if (m_Blink != 0.0f && ups::timeGet()-m_TimerBlink > ups::timeFreq()*m_Blink)
	{
		if (!m_Blinked && !upm::randInt(0,10))
			m_Blinked = true;
		else if (m_Blinked)
			m_Blinked = false;

		m_TimerBlink = ups::timeGet();
	}
}
