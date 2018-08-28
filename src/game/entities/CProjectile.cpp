#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include "CProjectile.hpp"
#include "CFire.hpp"
#include <engine/CConfig.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemSound.hpp>

const CB2BodyInfo CProjectile::ms_BodyInfo = CB2BodyInfo(0.2f, 1.25f, 0.1f, 0.0f, b2_dynamicBody, CAT_PROJECTILE, false, CAT_FIRE|CAT_BUILD|CAT_GENERIC|CAT_PROJECTILE|CAT_WATER);
CProjectile::CProjectile(const sf::Vector2f &pos, const sf::Vector2f &size, float rot, const sf::Vector2f &dir, float speed, class CPlayer *pOwner, unsigned int type, float lifeTime, unsigned int subtype) noexcept
: CB2Polygon(pos, size, rot, sf::Color::White, ms_BodyInfo, CEntity::PROJECTILE)
{
    m_pBody = getBody();
    m_Speed = speed;
    m_Dir = dir;
    m_pPlayer = pOwner;
    m_ProjType = type;
    m_LifeTime = lifeTime;
    m_ProjSubType = subtype;
    m_HasTouched = false;
    m_pEntTouch = nullptr;
    m_TimerSpawnFire = 0;

    if (m_pBody)
    {
    	m_pBody->SetBullet(true);
		m_pBody->SetTransform(m_pBody->GetPosition(), upm::degToRad(upm::vectorAngle(m_Dir)));
    }

    if (type == WEAPON_GRENADE_LAUNCHER)
    {
    	m_pBody->SetAngularVelocity(upm::floatRand(-8.0f, 8.0f));
    	m_pBody->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(m_Dir*m_Speed), true);
    }
    else if (type == WEAPON_CANON_BALL)
    {
    	b2Filter nFilter = m_pBody->GetFixtureList()[0].GetFilterData();
    	m_pBody->SetType(b2_kinematicBody);
    	nFilter.maskBits = CAT_FIRE|CAT_BUILD|CAT_GENERIC|CAT_PROJECTILE|CAT_CHARACTER_PLAYER;
    	m_pBody->GetFixtureList()[0].SetFilterData(nFilter);
    	getBody()->SetFixedRotation(true);
    }

    m_TickStart = ups::timeGet();
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

	if (m_ProjType == WEAPON_CANON_BALL)
	{
		const sf::Vector2f ProjPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		if (!m_pEntTouch)
		{
			if (ups::timeGet()-m_TimerSpawnFire > ups::timeFreq()*0.08f)
			{
				CGame *pGame = CGame::getInstance();
				pGame->Client()->Controller()->createFireTrailLarge(ProjPos-m_Dir*48.0f);
				m_TimerSpawnFire = ups::timeGet();
			}
			getBody()->SetLinearVelocity(CSystemBox2D::sfToB2(m_Dir*m_Speed));
		}
		else if (!m_HasTouched)
		{
			m_pBody->SetType(b2_dynamicBody);
			getBody()->SetFixedRotation(false);

			if (m_pEntTouch->getType() == CEntity::CHARACTER)
			{
				CCharacter *pChar = static_cast<CCharacter*>(m_pEntTouch);
				pChar->setMoveState(CCharacter::MOVE_STATE_UP);
			}
		}
	}

	// Self-Destruction
	const unsigned long elapsedTicks = ups::timeGet()-m_TickStart;
	if (m_LifeTime != 0.0f && elapsedTicks > ups::timeFreq()*m_LifeTime)
	{
		CGame *pGame = CGame::getInstance();
		const sf::Vector2f ProjPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		if (m_ProjType == WEAPON_GRENADE_LAUNCHER && !isToDelete())
		{
			pGame->Client()->Controller()->createImpactSparkMetal(ProjPos);
			pGame->Client()->Controller()->createExplosionCar(ProjPos, true);
			pGame->Client()->getSystem<CSystemBox2D>()->createExplosion(ProjPos, g_Config.m_aWeaponsInfo[m_ProjType].m_Energy, g_Config.m_aWeaponsInfo[m_ProjType].m_Radius, m_pBody);
			destroy();
		}
		else if (m_ProjType == WEAPON_CANON_BALL)
			pGame->Client()->Controller()->createExplosionCar(ProjPos, true);

		destroy();
	}

	m_HasTouched = (m_pEntTouch != nullptr);
}

void CProjectile::onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept
{
	CB2Polygon::onContact(pEntity, worldPos);

	if (pEntity->getType() == CEntity::FIRE)
		return;

	CGame *pGame = CGame::getInstance();
	if (m_ProjType == WEAPON_GRENADE_LAUNCHER && !isToDelete())
	{
		pGame->Client()->Controller()->createImpactSparkMetal(worldPos);
		pGame->Client()->Controller()->createExplosionCar(worldPos, true);
		pGame->Client()->getSystem<CSystemBox2D>()->createExplosion(worldPos, g_Config.m_aWeaponsInfo[m_ProjType].m_Energy, g_Config.m_aWeaponsInfo[m_ProjType].m_Radius, m_pBody);
		destroy();
	}
	else if (m_ProjType == WEAPON_CANON_BALL)
	{
		if (pEntity->getType() == CEntity::CHARACTER)
		{
			m_pEntTouch = pEntity;
		}
		else if (!isToDelete())
		{
			pGame->Client()->Controller()->createExplosionCar(worldPos, true);
			destroy();
		}
	}
	else
		destroy();
}
