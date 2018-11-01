/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/client/CClient.hpp>
#include "CB2Chain.hpp"

CB2Chain::CB2Chain(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> &points, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, color)
{
	CClient *pGame = CClient::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolyLineBody(worldPos, points, rot, bodyInfo);
	m_pBody->SetUserData(this);
}
CB2Chain::~CB2Chain() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Chain", "Deleted");
	#endif
}

void CB2Chain::tick() noexcept
{
	CClient *pGame = CClient::getInstance();
	if (canSelfDelete())
	{
		bool needDelete = true;
		b2ChainShape *pChainShape = static_cast<b2ChainShape*>(m_pBody->GetFixtureList()[0].GetShape());
		for (int i=0; i<pChainShape->m_count; ++i)
		{
			if (!pGame->Client()->isClipped(CSystemBox2D::b2ToSf(m_pBody->GetPosition()+pChainShape->m_vertices[i]), SCREEN_MARGIN_DESTRUCTION))
				needDelete = false;
		}
		if (needDelete)
			destroy();
	}
}

void CB2Chain::onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept
{
	if (m_ContactFx != CEntity::FX_NONE)
	{
		CClient *pGame = CClient::getInstance();
		if (m_ContactFx == CEntity::FX_SPARKS)
			pGame->Client()->Controller()->createImpactSparkMetal(worldPos);
	}
}
