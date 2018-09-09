/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include "CSign.hpp"
#include <engine/CSystemBox2D.hpp>

const CB2BodyInfo CSign::ms_BodyInfo = CB2BodyInfo(0.2f, 0.7f, 0.1f, 0.0f, b2_dynamicBody, CAT_SIGN, true, CAT_CHARACTER_PLAYER);
CSign::CSign(const sf::Vector2f &pos, const sf::Vector2f &size, float rot, const char *pText) noexcept
: CB2Polygon(pos, size, rot, sf::Color::Transparent, ms_BodyInfo, CEntity::SIGN)
{
	m_pBody = getBody();
	strncpy(m_aText, pText, TEXT_MAX_LENGTH);
}
CSign::~CSign() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSign", "Deleted");
	#endif
}

void CSign::tick() noexcept
{
	CB2Polygon::tick();
}


/**  SENSOR **/
void CSign::onSensorIn(CEntity *pEntity) noexcept
{
	CB2Polygon::onSensorIn(pEntity);
	CGame *pGame = CGame::getInstance();
	pGame->Client()->showHelpMessage(m_aText);
}
void CSign::onSensorOut(CEntity *pEntity) noexcept
{
	CB2Polygon::onSensorOut(pEntity);
	CGame *pGame = CGame::getInstance();
	pGame->Client()->showHelpMessage("");
}
