/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CZone.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>

const CB2BodyInfo CZone::ms_BodyInfo = CB2BodyInfo(0.0f, 0.0f, 0.0f, 0.0f, b2_staticBody, CAT_ZONE, true, CAT_CHARACTER_PLAYER|CAT_PROJECTILE|CAT_GENERIC|CAT_HITBOX|CAT_BUILD);
CZone::CZone(const sf::Vector2f &pos, const sf::Vector2f &size, float rot, float gravityScale) noexcept
: CB2Polygon(pos, size, rot, sf::Color::Transparent, ms_BodyInfo, CEntity::WATER)
{
	m_GravityScale = gravityScale;
	m_pFixture = &getBody()->GetFixtureList()[0];
}
CZone::~CZone() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CZone", "Deleted");
	#endif
}

void CZone::tick() noexcept
{

	CGame *pGame = CGame::getInstance();
	CSystemBox2D *pSystemBox2D = pGame->Client()->getSystem<CSystemBox2D>();
	// LiquidFun Particles
	b2AABB bounds;
	b2PolygonShape *pShape = static_cast<b2PolygonShape*>(m_pFixture->GetShape());
	bounds.lowerBound.Set(getBody()->GetPosition().x - pShape->GetVertex(2).x, getBody()->GetPosition().y - pShape->GetVertex(2).y);
	bounds.upperBound.Set(getBody()->GetPosition().x + pShape->GetVertex(2).x, getBody()->GetPosition().y + pShape->GetVertex(2).y);
	// FIXME: idk why need divide by 10
	const b2Vec2 force(pSystemBox2D->getWorld()->GetGravity() * m_GravityScale/10.0f);
	MultiQueryParticleSystemCallBack callbackParticleSystem;
	for (b2ParticleSystem *pParticleSystem = pSystemBox2D->getWorld()->GetParticleSystemList(); pParticleSystem; pParticleSystem = pParticleSystem->GetNext())
	{
		pParticleSystem->QueryAABB(&callbackParticleSystem, bounds);
		if (callbackParticleSystem.m_vIndex.size())
		{
			for (std::size_t i=0; i<callbackParticleSystem.m_vIndex.size(); ++i)
			{
				b2Vec2 *pParticleVel = pParticleSystem->GetVelocityBuffer();
				b2Vec2 *pCurParticleVel = (pParticleVel+callbackParticleSystem.m_vIndex[i]);
				pCurParticleVel->Set(pCurParticleVel->x + force.x, pCurParticleVel->y + force.y);
			}
		}
	}
}

void CZone::onSensorIn(CEntity *pEntity) noexcept
{
	pEntity->getBody()->SetGravityScale(m_GravityScale);
}

void CZone::onSensorOut(CEntity *pEntity) noexcept
{
	pEntity->getBody()->SetGravityScale(1.0f);
}
