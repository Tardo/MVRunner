/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/entities/CHitBox.hpp>
#include <game/entities/CProjectile.hpp>
#include "CItemRender.hpp"

CItemRender::CItemRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CItemRender::~CItemRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CItemRender", "Deleted");
	#endif
}

void CItemRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context())
		return;

	target.setView(Client()->Camera());

	std::vector<CEntity*> &vEntities = Client()->Controller()->Context()->getEntities();
	std::vector<CEntity*>::const_iterator cit = vEntities.cbegin();
	while (cit != vEntities.cend())
	{
		if ((*cit)->getType() == CEntity::B2POLY || (*cit)->getType() == CEntity::B2CIRCLE || (*cit)->getType() == CEntity::B2CHAIN)
			renderPrimitive(target, states, (*cit));
		else if ((*cit)->getType() == CEntity::PROJECTILE)
			renderProjectile(target, states, (*cit));
		else if ((*cit)->getType() == CEntity::HITBOX)
			renderHitBox(target, states, (*cit));
		++cit;
	}
}

void CItemRender::renderHitBox(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept
{

}

void CItemRender::renderProjectile(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept
{
	if (Client()->getRenderMode() != CGameClient::RENDER_NORMAL)
		return;

	CProjectile *pProj = static_cast<CProjectile*>(pEntity);
	b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
	sf::ConvexShape Shape(pB2Poly->m_count);
	for (int i=0; i<pB2Poly->m_count; ++i)
		Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
	Shape.setPosition(CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()));
	Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
	if (pProj->getProjType() == WEAPON_GRENADE_LAUNCHER)
		Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_GRENADE));

	target.draw(Shape, states);
}

void CItemRender::renderPrimitive(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept
{
	if (pEntity->m_Color == sf::Color::Transparent)
		return;

	if (pEntity->getType() == CEntity::B2POLY)
	{
		b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		sf::ConvexShape Shape(pB2Poly->m_count);
		for (int i=0; i<pB2Poly->m_count; ++i)
			Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
		Shape.setPosition(CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()));
		Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
		Shape.setFillColor(pEntity->m_Color);

		target.draw(Shape, states);
	}
	else if (pEntity->getType() == CEntity::B2CIRCLE)
	{
		b2CircleShape *pB2Circle = static_cast<b2CircleShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		sf::CircleShape Shape(pB2Circle->m_radius);
		Shape.setPosition(CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()));
		Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
		Shape.setFillColor(pEntity->m_Color);

		target.draw(Shape, states);
	}
	else if (pEntity->getType() == CEntity::B2CHAIN)
	{
		b2ChainShape *pB2Chain = static_cast<b2ChainShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		sf::VertexArray VArray(sf::LineStrip, pB2Chain->m_count);
		for (int i=0; i<pB2Chain->m_count; ++i)
		{
			VArray[i].color = pEntity->m_Color;
			VArray[i].position = CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition() + pB2Chain->m_vertices[i]);
		}
		target.draw(VArray, states);
	}
}
