/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/entities/CWater.hpp>
#include "CFluidRender.hpp"

CFluidRender::CFluidRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CFluidRender::~CFluidRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CFluidRender", "Deleted");
	#endif
}

void CFluidRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context())
		return;

	target.setView(Client()->Camera());

	std::vector<CEntity*> &vEntities = Client()->Controller()->Context()->getEntities();
	std::vector<CEntity*>::const_iterator cit = vEntities.cbegin();
	while (cit != vEntities.cend())
	{
		if ((*cit)->getType() == CEntity::WATER)
			renderWater(target, states, (*cit));
		++cit;
	}
}

void CFluidRender::renderWater(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept
{
	if (pEntity->m_Color == sf::Color::Transparent)
		return;

	const sf::Vector2f waterPos = CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition());

	b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
	sf::ConvexShape Shape(pB2Poly->m_count);
	for (int i=0; i<pB2Poly->m_count; ++i)
		Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
	Shape.setPosition(waterPos);
	Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
	Shape.setFillColor(pEntity->m_Color);
	target.draw(Shape, states);
}
