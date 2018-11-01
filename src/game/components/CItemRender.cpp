/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/client/CClient.hpp>
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
	if (!Client()->Controller() || !Client()->Controller()->Context() || Client()->getRenderMode() != RENDER_MODE_NORMAL)
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
	if (Client()->getRenderMode() != RENDER_MODE_NORMAL)
		return;

	const sf::Vector2f projPos = CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition());
	if (Client()->isClipped(projPos, SCREEN_MARGIN_DRAW))
		return;

	CProjectile *pProj = static_cast<CProjectile*>(pEntity);
	b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
	sf::ConvexShape Shape(pB2Poly->m_count);
	for (int i=0; i<pB2Poly->m_count; ++i)
		Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
	Shape.setPosition(projPos);
	Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
	if (pProj->getProjType() == WEAPON_GRENADE_LAUNCHER)
		Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_GRENADE));
	else if (pProj->getProjType() == WEAPON_CANON_BALL)
		Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_CANON_BALL));
	else if (pProj->getProjType() == WEAPON_VISCOSITY_LAUNCHER)
		Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_GRENADE));

	if (pProj->getDir().x < 0.0f)
		Shape.setTextureRect(sf::IntRect(0, Shape.getTexture()->getSize().y, Shape.getTexture()->getSize().x, -static_cast<int>(Shape.getTexture()->getSize().y)));
	if (pProj->getDir().y < 0.0f)
			Shape.setTextureRect(sf::IntRect(Shape.getTexture()->getSize().x, 0, -static_cast<int>(Shape.getTexture()->getSize().x), Shape.getTexture()->getSize().y));

	target.draw(Shape, states);
}

void CItemRender::renderPrimitive(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept
{
	if (pEntity->m_Color == sf::Color::Transparent)
		return;

	// TODO: Check if need render by b2type
	if (pEntity->getType() == CEntity::B2POLY)
	{
		b2PolygonShape *pB2Poly = static_cast<b2PolygonShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		sf::ConvexShape Shape(pB2Poly->m_count);
		for (int i=0; i<pB2Poly->m_count; ++i)
			Shape.setPoint(i, CSystemBox2D::b2ToSf(pB2Poly->m_vertices[i]));
		Shape.setPosition(CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()));
		Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
		Shape.setFillColor(pEntity->m_Color);
		if (pEntity->m_TextureId != -1)
			Shape.setTexture(Client()->Assets().getTexture(pEntity->m_TextureId));

		target.draw(Shape, states);
	}
	else if (pEntity->getType() == CEntity::B2CIRCLE)
	{
		b2CircleShape *pB2Circle = static_cast<b2CircleShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		const int radius = CSystemBox2D::b2ToSf(pB2Circle->m_radius);
		sf::CircleShape Shape(radius, 16);
		Shape.setOrigin(radius, radius);
		Shape.setPosition(CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()));
		Shape.setRotation(upm::radToDeg(pEntity->getBody()->GetAngle()));
		Shape.setFillColor(pEntity->m_Color);
		if (pEntity->m_TextureId != -1)
			Shape.setTexture(Client()->Assets().getTexture(pEntity->m_TextureId));

		target.draw(Shape, states);
	}
	else if (pEntity->getType() == CEntity::B2CHAIN)
	{
		b2ChainShape *pB2Chain = static_cast<b2ChainShape*>(pEntity->getBody()->GetFixtureList()[0].GetShape());
		sf::VertexArray VArray(sf::LineStrip, pB2Chain->m_count);
		for (int i=0; i<pB2Chain->m_count; ++i)
		{
			VArray[i].color = pEntity->m_Color;
			VArray[i].position = CSystemBox2D::b2ToSf(pEntity->getBody()->GetPosition()) + CSystemBox2D::b2ToSf(pB2Chain->m_vertices[i]);
		}
		target.draw(VArray, states);
	}
}
