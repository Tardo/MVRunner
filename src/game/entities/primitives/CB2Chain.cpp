/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include "CB2Chain.hpp"

CB2Chain::CB2Chain(sf::Vector2f worldPos, int zlevel, const std::vector<sf::Vector2f> points, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, zlevel),
  m_LineShape(sf::LineStrip, points.size())
{
	m_IsSensor = bodyInfo.m_IsSensor;
	m_Color = color;

	for (unsigned i=0; i<m_LineShape.getVertexCount(); ++i)
	{
		m_LineShape[i].color = m_Color;
		m_LineShape[i].position = worldPos+points[i];
	}

	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->m_Debug)
	{
		m_DbgShape = m_LineShape;
		for (unsigned int i=0; i<m_DbgShape.getVertexCount(); i++)
			m_DbgShape[i].color = sf::Color::Magenta;
	}

	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolyLineBody(worldPos, points, bodyInfo);
	m_pBody->SetUserData(this);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CB2Chain::~CB2Chain() noexcept
{
	CGame *pGame = CGame::getInstance();
	pGame->Client()->getSystem<CSystemBox2D>()->destroyBody(m_pBody);
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Chain", "Deleted");
	#endif
}

void CB2Chain::tick() noexcept
{
	const b2Vec2 &b2ShapePos = m_pBody->GetPosition();
	b2ChainShape *pB2Shape = static_cast<b2ChainShape*>(m_pBody->GetFixtureList()[0].GetShape());
	for (unsigned i=0; i<m_LineShape.getVertexCount(); ++i)
		m_LineShape[i].position = CSystemBox2D::b2ToSf(b2ShapePos+pB2Shape->m_vertices[i]);
}

void CB2Chain::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CGame *pGame = CGame::getInstance();

	if (m_pBody && m_pBody->IsActive())
	{
		if (pGame->Client()->m_Debug)
		{
			target.draw(m_DbgShape, states);
		} else if (m_Color != sf::Color::Transparent)
			target.draw(m_LineShape, states);
	}
}

