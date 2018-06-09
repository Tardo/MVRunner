/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include "CB2Circle.hpp"

CB2Circle::CB2Circle(const sf::Vector2f &worldPos, float radius, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createCircleBody(worldPos, radius, rot, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Circle::~CB2Circle() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Circle", "Deleted");
	#endif
}

void CB2Circle::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->isClipped(CSystemBox2D::b2ToSf(m_pBody->GetPosition()), SCREEN_MARGIN_DESTRUCTION))
		destroy();
}
