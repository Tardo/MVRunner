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
	renderBox2DBodies(target, states);
	renderBox2DJoints(target, states);

	if (Client()->Controller() && Client()->Controller()->Context())
		renderQuadTree(target, states, Client()->Controller()->Context()->Map().getObjects());
}

void CDebuggerRender::renderBox2DJoints(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CSystemBox2D *pSystemBox2D = Client()->getSystem<CSystemBox2D>();
	for (b2Joint *pJoint = pSystemBox2D->getWorld()->GetJointList(); pJoint; pJoint = pJoint->GetNext())
	{
		sf::VertexArray VArray(sf::LineStrip, 2);
		VArray[0].color = sf::Color::Magenta;
		VArray[0].position = CSystemBox2D::b2ToSf(pJoint->GetAnchorA());
		VArray[1].color = sf::Color::Magenta;
		VArray[1].position = CSystemBox2D::b2ToSf(pJoint->GetAnchorB());
		target.draw(VArray, states);
	}
}

void CDebuggerRender::renderBox2DBodies(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CSystemBox2D *pSystemBox2D = Client()->getSystem<CSystemBox2D>();
	for (b2Body *pBody = pSystemBox2D->getWorld()->GetBodyList(); pBody; pBody = pBody->GetNext())
	{
		const sf::Vector2f bodyPos = CSystemBox2D::b2ToSf(pBody->GetPosition());
		for (b2Fixture *pFixture = pBody->GetFixtureList(); pFixture; pFixture = pFixture->GetNext())
		{
			const sf::FloatRect localBounds = CSystemBox2D::b2ToSf(pFixture->GetAABB(0));
			const sf::FloatRect globalBounds(
				pBody->GetPosition().x + localBounds.left,
				pBody->GetPosition().y + localBounds.top,
				localBounds.width,
				localBounds.height
			);

			if (Client()->isClipped(globalBounds, SCREEN_MARGIN_DRAW))
				continue;

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

void CDebuggerRender::renderQuadTree(sf::RenderTarget& target, sf::RenderStates states, CQuadTree<CMapRenderObject*> *pQuadTree) const noexcept
{
	if (!pQuadTree)
		return;

	const sf::FloatRect &bounds = pQuadTree->getBounds();
	if (Client()->isClipped(bounds, SCREEN_MARGIN_DRAW))
		return;

	sf::RectangleShape Shape(sf::Vector2f(bounds.width, bounds.height));
	Shape.setPosition(bounds.left, bounds.top);
	Shape.setFillColor(sf::Color::Transparent);
	Shape.setOutlineColor(sf::Color::Red);
	Shape.setOutlineThickness(2.0f);
	target.draw(Shape);
	for (int i=0; i<pQuadTree->getNumItems(); ++i)
	{
		sf::CircleShape Circle(10.0f);
		Circle.setPosition(pQuadTree->getItem(i)->m_Pos);
		Circle.setOrigin(10.0f, 10.0f);
		Circle.setFillColor(sf::Color::Red);
		target.draw(Circle);
	}
	renderQuadTree(target, states, pQuadTree->m_pNW);
	renderQuadTree(target, states, pQuadTree->m_pNE);
	renderQuadTree(target, states, pQuadTree->m_pSW);
	renderQuadTree(target, states, pQuadTree->m_pSE);
}
