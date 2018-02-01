/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include "CB2Polygon.hpp"

CB2Polygon::CB2Polygon(sf::Vector2f worldPos, int zlevel, const std::vector<sf::Vector2f> points, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, zlevel)
{
	m_IsSensor = bodyInfo.m_IsSensor;
	sf::ConvexShape *pConvex = new sf::ConvexShape(points.size());
	for (size_t i=0; i<points.size(); i++)
		pConvex->setPoint(i, points[i]);
	m_pShape = pConvex;
	m_pShape->setFillColor(color);
	m_pShape->setPosition(worldPos);

	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createPolygonBody(worldPos, points, bodyInfo);
	m_pBody->SetUserData(this);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CB2Polygon::CB2Polygon(sf::Vector2f worldPos, int zlevel, const sf::Vector2f size, const sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, zlevel)
{
	m_IsSensor = bodyInfo.m_IsSensor;

	m_pShape = new sf::RectangleShape(size);
	m_pShape->setFillColor(color);
	m_pShape->setOrigin(size.x/2.0f, size.y/2.0f);
	m_pShape->setPosition(worldPos);

	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createBoxBody(worldPos, size, bodyInfo);
	m_pBody->SetUserData(this);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CB2Polygon::~CB2Polygon() noexcept
{
	CGame *pGame = CGame::getInstance();
	pGame->Client()->getSystem<CSystemBox2D>()->destroyBody(m_pBody);
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Box", "Deleted");
	#endif
}

void CB2Polygon::tick() noexcept
{
	if (m_pBody)
	{
		m_pShape->setPosition(CSystemBox2D::b2ToSf(m_pBody->GetPosition()));
		m_pShape->setRotation(upm::radToDeg(m_pBody->GetAngle()));
	}

	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->m_Debug)
	{
		m_DbgShape = *(static_cast<sf::RectangleShape*>(getShape()));
		m_DbgShape.setTexture(0x0);
		m_DbgShape.setFillColor(sf::Color::Blue);
		m_DbgShape.setOutlineThickness(1.0f);
		m_DbgShape.setOutlineColor(sf::Color::Magenta);
	}
}

void CB2Polygon::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CGame *pGame = CGame::getInstance();
	if (m_pBody && m_pBody->IsActive())
	{
		if (pGame->Client()->m_Debug)
		{
			target.draw(m_DbgShape, states);

			// Direction
			const sf::Vertex lineDir[] =
			{
				sf::Vertex(m_pShape->getPosition(), sf::Color::Red),
				sf::Vertex(m_pShape->getPosition()+upm::vectorNormalize(CSystemBox2D::b2ToSf(m_pBody->GetLinearVelocity())), sf::Color::Red)
			};
			target.draw(lineDir, 2, sf::Lines, states);

			// Velocity
			const sf::Vertex lineVel[] =
			{
				sf::Vertex(m_pShape->getPosition(), sf::Color::Green),
				sf::Vertex(m_pShape->getPosition()+CSystemBox2D::b2ToSf(m_pBody->GetLinearVelocity()), sf::Color::Green)
			};
			target.draw(lineVel, 2, sf::Lines, states);
		}
		else if (m_pShape->getFillColor() != sf::Color::Transparent)
		{
			target.draw(*m_pShape, states);
		}
	}
}

