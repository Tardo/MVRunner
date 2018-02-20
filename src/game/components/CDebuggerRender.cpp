/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/entities/CHitBox.hpp>
#include <game/entities/CProjectile.hpp>
#include "CDebuggerRender.hpp"

CDebuggerRender::CDebuggerRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CDebuggerRender::~CDebuggerRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CDebuggerRender", "Deleted");
	#endif
}

void CDebuggerRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->m_Debug)
		return;

	target.setView(Client()->Camera());
	renderBox2D(target, states);
}

void CDebuggerRender::renderBox2D(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CSystemBox2D *pSystemBox2D = Client()->getSystem<CSystemBox2D>();
	for (b2Body *pBody = pSystemBox2D->getWorld()->GetBodyList(); pBody; pBody = pBody->GetNext())
	{
		const sf::Vector2f bodyPos = CSystemBox2D::b2ToSf(pBody->GetPosition());
		sf::FloatRect screenArea;
		Client()->getViewportGlobalBounds(&screenArea, Client()->Camera());
		if (!screenArea.contains(bodyPos))
			continue;

		for (b2Fixture *pFixture = pBody->GetFixtureList(); pFixture; pFixture = pFixture->GetNext())
		{
			sf::Color color = sf::Color::Blue;
			if (pFixture->IsSensor())
				color.a = 64.0f;
			if (!pBody->IsAwake())
			{
				color.r -= 8.0f;
				color.g -= 8.0f;
				color.b -= 8.0f;
			}

			if (pFixture->GetShape()->GetType() == b2Shape::e_polygon)
			{
				b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pFixture->GetShape());
				sf::ConvexShape Shape(pB2Poly->m_count);
				for (int i=0; i<pB2Poly->m_count; ++i)
					Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
				Shape.setPosition(bodyPos);
				Shape.setRotation(upm::radToDeg(pBody->GetAngle()));
				Shape.setFillColor(color);
				Shape.setOutlineThickness(1.0f);
				Shape.setOutlineColor(sf::Color::Magenta);

				target.draw(Shape, states);
			}
			else if (pFixture->GetShape()->GetType() == b2Shape::e_circle)
			{
				b2CircleShape *pB2Circle = static_cast<b2CircleShape*>(pFixture->GetShape());
				sf::CircleShape Shape(pB2Circle->m_radius*PPM, 10);
				Shape.setPosition(bodyPos);
				Shape.setRotation(upm::radToDeg(pBody->GetAngle()));
				Shape.setOrigin(pB2Circle->m_radius*PPM, pB2Circle->m_radius*PPM);
				Shape.setFillColor(color);
				Shape.setOutlineThickness(1.0f);
				Shape.setOutlineColor(sf::Color::Magenta);

				target.draw(Shape, states);
			}
			else if (pFixture->GetShape()->GetType() == b2Shape::e_chain)
			{
				b2ChainShape *pB2Chain = static_cast<b2ChainShape*>(pFixture->GetShape());
				sf::VertexArray VArray(sf::LineStrip, pB2Chain->m_count);
				for (int i=0; i<pB2Chain->m_count; ++i)
				{
					VArray[i].color = sf::Color::Magenta;
					VArray[i].position = CSystemBox2D::b2ToSf(pBody->GetPosition() + pB2Chain->m_vertices[i]);
				}
				target.draw(VArray, states);
			}
		}
	}
}
