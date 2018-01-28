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
	m_ShadowSizeFactor = 1.1f;
	m_NumShadows = 0;

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

	// Clean Shadows
	m_NumShadows = 0;

	// Sun Shadow
	if (!m_IsSensor && getZLevel())
	{
		const sf::Vector2f &shapePos = m_pShape->getPosition();
		const sf::Texture *pTexture = getShape()->getTexture();
		if (pTexture)
		{
			CSystemLight *pLight = pGame->Client()->getSystem<CSystemLight>();
			sf::Sprite Shadow;
			Shadow.setTexture(*pTexture, true);
			Shadow.setTextureRect(getShape()->getTextureRect());
			Shadow.setOrigin(sf::Vector2f(Shadow.getTextureRect().width/2.0f, Shadow.getTextureRect().height/2.0f));
			Shadow.setColor(pLight->getShadowColor());
			Shadow.setRotation(upm::radToDeg(m_pBody->GetAngle()));
			Shadow.setScale(m_ShadowSizeFactor, m_ShadowSizeFactor);
			Shadow.setPosition(shapePos+pLight->getShadowOffset());
			m_aShadows[m_NumShadows++] = Shadow;
			// Lights Shadows
			std::vector<CLight*> vpNearLights = pGame->Client()->getSystem<CSystemLight>()->getNearLights(shapePos);
			std::vector<CLight*>::const_iterator itl = vpNearLights.begin();
			while (itl != vpNearLights.end())
			{
				if (m_NumShadows == MAX_SHADOWS)
					break;

				CLight *pLight = (*itl);
				sf::Vector2f dir = shapePos - pLight->m_Position;
				const float dist = upm::vectorLength(dir);
				dir = upm::vectorNormalize(dir);

				sf::Color shadowColor = pGame->Client()->getSystem<CSystemLight>()->getShadowColor();
				shadowColor.a = 90 - upm::clamp(dist, 0.0f, 90.0f);
				Shadow.setColor(shadowColor);
				Shadow.setPosition(shapePos+dir*dist*0.1f);
				m_aShadows[m_NumShadows++] = Shadow;

				++itl;
			}
		}
	}
}

void CB2Circle::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CGame *pGame = CGame::getInstance();
	if (m_pBody && m_pBody->IsActive())
	{
		// Shadow
		if (!m_IsSensor && getZLevel())
		{
			for (unsigned int i=0; i<m_NumShadows; target.draw(m_aShadows[i++], states));
		}

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
