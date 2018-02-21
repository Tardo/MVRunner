/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include "CB2Polygon.hpp"

CB2Polygon::CB2Polygon(sf::Vector2f worldPos, const std::vector<sf::Vector2f> points, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolygonBody(worldPos, points, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Polygon::CB2Polygon(sf::Vector2f worldPos, const sf::Vector2f size, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createBoxBody(worldPos, size, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Polygon::~CB2Polygon() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Polygon", "Deleted");
	#endif
}

void CB2Polygon::tick() noexcept
{ }
