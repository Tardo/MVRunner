/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include "CB2Chain.hpp"

CB2Chain::CB2Chain(sf::Vector2f worldPos, const std::vector<sf::Vector2f> points, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolyLineBody(worldPos, points, bodyInfo);
	m_pBody->SetUserData(this);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CB2Chain::~CB2Chain() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Chain", "Deleted");
	#endif
}

void CB2Chain::tick() noexcept
{ }
