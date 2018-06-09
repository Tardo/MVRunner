/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include "CB2Polygon.hpp"

CB2Polygon::CB2Polygon(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> &points, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolygonBody(worldPos, points, rot, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Polygon::CB2Polygon(const sf::Vector2f &worldPos, const sf::Vector2f &size, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createBoxBody(worldPos, size, rot, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Polygon::~CB2Polygon() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Polygon", "Deleted");
	#endif
}

void CB2Polygon::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	bool needDelete = true;
	b2PolygonShape *pPolyShape = static_cast<b2PolygonShape*>(m_pBody->GetFixtureList()[0].GetShape());
	for (int i=0; i<pPolyShape->m_count; ++i)
	{
		if (!pGame->Client()->isClipped(CSystemBox2D::b2ToSf(m_pBody->GetPosition()+pPolyShape->m_vertices[i]), SCREEN_MARGIN_DESTRUCTION))
			needDelete = false;
	}
	if (needDelete)
		destroy();
}
