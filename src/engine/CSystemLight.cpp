/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CSystemLight.hpp>
#include "CGame.hpp"
#include <cstring>

CSystemLight::CSystemLight() noexcept
{
	m_ShadowOffset = sf::Vector2f(2.5f, 1.25f);
	m_ColorClear = sf::Color(128, 128, 128);
	m_ShadowColor = sf::Color(64, 64, 64);
}
CSystemLight::~CSystemLight() noexcept
{
	std::list<CLight*>::iterator itl = m_vpLights.begin();
	while (itl != m_vpLights.end())
	{
		delete (*itl);
		++itl;
	}
	m_vpLights.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemLight", "Deleted");
	#endif
}

bool CSystemLight::init(class CGameClient *pGameClient) noexcept
{
	CSystem::init(pGameClient);

	if (!m_LightMapTexture.create(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight))
		return false;
	m_Lightmap.setTexture(m_LightMapTexture.getTexture());

	return true;
}

void CSystemLight::update(float deltaTime) noexcept
{

}

sf::Color CSystemLight::getTimeColor() noexcept
{
	return m_ColorClear;
}

const sf::Sprite& CSystemLight::getLightmap(const sf::View &camera) noexcept
{
	m_LightMapTexture.clear(getTimeColor());
	m_LightMapTexture.setView(camera);

	std::list<CLight*>::iterator itl = m_vpLights.begin();
	while (itl != m_vpLights.end())
	{
		CLight *pLight = static_cast<CLight*>(*itl);
		if (pLight->m_ToDelete) // Not need use "update" method.. less loops, more fast
		{
			delete (*itl);
			itl = m_vpLights.erase(itl);
			continue;
		}

		// Parpadeos
		if (pLight->m_Blink != 0.0f && ups::timeGet()-pLight->m_TimerBlink > ups::timeFreq()*pLight->m_Blink)
		{
			if (!pLight->m_Blinked && !upm::randInt(0,10))
				pLight->m_Blinked = true;
			else if (pLight->m_Blinked)
				pLight->m_Blinked = false;

			pLight->m_TimerBlink = ups::timeGet();
		}

		// Render Light
		if (pLight->m_Active && !pLight->m_Blinked)
		{
			if (pLight->m_MaskTextID != -1)
			{
				sf::Texture *pText = Client()->Assets().getTexture(pLight->m_TextID);
				sf::Sprite sprite = sf::Sprite(*pText);
				sf::Sprite spriteMask = sf::Sprite(*Client()->Assets().getTexture(pLight->m_MaskTextID));
				sf::RenderTexture renderText;
				renderText.create(pText->getSize().x, pText->getSize().y);
				renderText.draw(sprite);
				renderText.draw(spriteMask);
				sf::Sprite spriteText(renderText.getTexture());

				m_LightMapTexture.draw(spriteText, sf::BlendAdd);
			}
			else
			{
				const sf::Texture *pText = pLight->m_Sprite.getTexture();
				pLight->m_Sprite.setOrigin(pText->getSize().x*pLight->m_Origin.x, pText->getSize().y*pLight->m_Origin.y);
				float factorSize = 1.0f;
				if (pLight->m_VariationSize != 0.0f)
					factorSize = upm::floatRand(1.0f, pLight->m_VariationSize);
				pLight->m_Sprite.setScale(pLight->m_Scale*factorSize);
				pLight->m_Sprite.setColor(pLight->m_Color);
				pLight->m_Sprite.setPosition(pLight->m_Position);
				pLight->m_Sprite.setRotation(pLight->m_Degrees);
				m_LightMapTexture.draw(pLight->m_Sprite, sf::BlendAdd);
			}
		}
		++itl;
	}
	m_LightMapTexture.display();

	return m_Lightmap;
}

std::vector<CLight*> CSystemLight::getNearLights(const sf::Vector2f &worldPos) noexcept
{
	std::vector<CLight*> vpLights;

	std::list<CLight*>::iterator itl = m_vpLights.begin();
	while (itl != m_vpLights.end())
	{
		CLight *pLight = static_cast<CLight*>(*itl);
		if (pLight->m_Active && pLight->m_MaskTextID == -1 && pLight->m_Sprite.getGlobalBounds().contains(worldPos))
			vpLights.push_back(pLight);

		++itl;
	}

	return vpLights;
}


CLight* CSystemLight::createPoint(const sf::Vector2f &worldPos, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(CAssetManager::TEXTURE_POINT_LIGHT, worldPos, 0.0f, scale, color, alwaysOn, blink, variationSize);
	pLight->m_Origin = sf::Vector2f(0.5f, 0.5f);
	m_vpLights.push_back(pLight);
	return pLight;
}

CLight* CSystemLight::createSpot(const sf::Vector2f &worldPos, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(CAssetManager::TEXTURE_SPOT_LIGHT, worldPos, degrees, scale, color, alwaysOn, blink, variationSize);
	pLight->m_Origin = sf::Vector2f(0.5f, 1.0f);
	m_vpLights.push_back(pLight);
	return pLight;
}

CLight* CSystemLight::createEmissive(const sf::Vector2f &worldPos, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(CAssetManager::TEXTURE_EMISSIVE_LIGHT, worldPos, degrees, scale, color, alwaysOn, blink, variationSize);
	pLight->m_Origin = sf::Vector2f(0.5f, 0.5f);
	m_vpLights.push_back(pLight);
	return pLight;
}

CLight* CSystemLight::createCustom(int textId, const sf::Vector2f &worldPos, const sf::Vector2f &origin, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(textId, worldPos, degrees, scale, color, alwaysOn, blink, variationSize);
	pLight->m_Origin = origin;
	m_vpLights.push_back(pLight);
	return pLight;
}

/** LIGHT **/
CLight::CLight(int textId, const sf::Vector2f &nposition, float angle, const sf::Vector2f &nscale, const sf::Color &ncolor, bool alwaysOn, float blink, float variationSize, int render) noexcept
: m_Position(nposition),
  m_Scale(nscale),
  m_Color(ncolor),
  m_Blink(blink),
  m_Degrees(angle),
  m_Render(render),
  m_AlwaysOn(alwaysOn),
  m_TextID(textId)
{
	m_Origin = VECTOR_ZERO;
	m_Active = true;
	m_ToDelete = false;
	m_Blinked = false;
	m_MaskTextID = -1;
	m_TimerBlink = 0;
	m_VariationSize = variationSize;

	if (m_TextID != -1)
	{
		CGame *pGame = CGame::getInstance();
		m_Sprite.setTexture(*pGame->Client()->Assets().getTexture(m_TextID));
	}
}
CLight::~CLight() noexcept
{ }
