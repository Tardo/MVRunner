/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include "CB2Circle.hpp"

CB2Circle::CB2Circle(sf::Vector2f worldPos, int zlevel, float radius, sf::Color color, const CB2BodyInfo &bodyInfo, int entityType) noexcept
: CEntity(entityType, zlevel)
{
	m_IsSensor = bodyInfo.m_IsSensor;
	m_pShape = new sf::CircleShape(radius, 10); // Low Definition
	m_pShape->setFillColor(color);
	m_pShape->setOrigin(radius, radius);
	m_pShape->setPosition(worldPos);

	CGame *pGame = CGame::getInstance();
	m_pBody = pGame->Client()->getSystem<CSystemBox2D>()->createCircleBody(worldPos, radius, bodyInfo);
	m_pBody->SetUserData(this);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CB2Circle::~CB2Circle() noexcept
{
	CGame *pGame = CGame::getInstance();
	pGame->Client()->getSystem<CSystemBox2D>()->destroyBody(m_pBody);
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CB2Circle", "Deleted");
	#endif
}

void CB2Circle::tick() noexcept
{
	if (m_pBody)
	{
		m_pShape->setPosition(CSystemBox2D::b2ToSf(m_pBody->GetPosition()));
		m_pShape->setRotation(upm::radToDeg(m_pBody->GetAngle()));
	}

	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->m_Debug)
	{
		m_DbgShape = *(static_cast<sf::CircleShape*>(getShape()));
		m_DbgShape.setTexture(0x0);
		m_DbgShape.setFillColor(sf::Color::Blue);
		m_DbgShape.setOutlineThickness(1.0f);
		m_DbgShape.setOutlineColor(sf::Color::Magenta);
	}
}

void CB2Circle::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CGame *pGame = CGame::getInstance();
	if (m_pBody && m_pBody->IsActive())
	{
		if (pGame->Client()->m_Debug)
		{
			target.draw(m_DbgShape, states);

			sf::Vector2f point = m_pShape->getPosition()+sf::Vector2f(0.0f, -1.0f)*16.0f;
			upm::vectorRotate(m_pShape->getPosition(), &point, upm::degToRad(m_pShape->getRotation()));
			// Direction
			const sf::Vertex lineDir[] =
			{
				sf::Vertex(m_pShape->getPosition(), sf::Color::Red),
				sf::Vertex(point, sf::Color::Red)
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
