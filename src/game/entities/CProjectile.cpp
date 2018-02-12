#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include "CProjectile.hpp"
#include "CFire.hpp"
#include <engine/CConfig.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemSound.hpp>

const CB2BodyInfo CProjectile::ms_BodyInfo = CB2BodyInfo(0.2f, 0.7f, 0.1f, b2_dynamicBody, CAT_PROJECTILE, false, CAT_FIRE|CAT_BUILD|CAT_GENERIC);
CProjectile::CProjectile(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float speed, class CPlayer *pOwner, unsigned int type, unsigned int subtype) noexcept
: CB2Polygon(pos, size, sf::Color::White, ms_BodyInfo, CEntity::PROJECTILE)
{
    m_pBody = getBody();
    m_Speed = speed;
    m_Dir = dir;
    m_pPlayer = pOwner;
    m_ProjType = type;
    m_ProjSubType = subtype;

    if (m_pBody)
    	m_pBody->SetBullet(true);

    CGame *pGame = CGame::getInstance();
    m_pSystemFx = pGame->Client()->getSystem<CSystemFx>();
    if (m_ProjType == WEAPON_GRENADE_LAUNCHER)
    {
    	if (m_pBody)
		{
			m_pBody->SetTransform(m_pBody->GetPosition(), upm::degToRad(upm::vectorAngle(m_Dir)));
			m_pBody->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(m_Dir*m_Speed), true);
			m_pBody->SetAngularVelocity(upm::floatRand(-8.0f, 8.0f));
		}
    }
}

CProjectile::~CProjectile() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CProjectile", "Deleted");
	#endif
}

void CProjectile::tick() noexcept
{
	CB2Polygon::tick();
	//const sf::Vector2f &shapePos = getShape()->getPosition();
}

void CProjectile::onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept
{
	CB2Polygon::onContact(pEntity, worldPos);

	if (pEntity->getType() == CEntity::FIRE)
		return;

	CGame *pGame = CGame::getInstance();
	if (!isToDelete() && m_ProjType == WEAPON_GRENADE_LAUNCHER)
	{
		pGame->Client()->getSystem<CSystemFx>()->createImpactSparkMetal(worldPos);
		pGame->Client()->getSystem<CSystemFx>()->createExplosionCar(worldPos, true);
		pGame->Client()->getSystem<CSystemBox2D>()->createExplosion(worldPos, g_Config.m_aWeaponsInfo[m_ProjType].m_Energy, g_Config.m_aWeaponsInfo[m_ProjType].m_Radius, m_pBody);
	}

	destroy();
}
